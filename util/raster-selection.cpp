// -*- coding: us-ascii-unix -*-
// Copyright 2012 Lukas Kemmer
//
// Licensed under the Apache License, Version 2.0 (the "License"); you
// may not use this file except in compliance with the License. You
// may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied. See the License for the specific language governing
// permissions and limitations under the License.

#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/draw.hh"
#include "geo/geo-func.hh"
#include "geo/tri.hh"
#include "rendering/faint-dc.hh"
#include "rendering/overlay.hh"
#include "util/raster-selection.hh"
#include "util/setting-util.hh"

namespace faint{

SelectionOptions::SelectionOptions(bool mask, const Paint& bg, bool alpha)
  : alpha(alpha),
    bg(bg),
    mask(mask)
{}

// A floating pasted bitmap
SelectionState::SelectionState(const Bitmap& bmp, const IntPoint& topLeft)
  : copy(true),
    floating(true),
    floatingBmp(bmp),
    oldRect(IntRect()),
    rect(IntRect(topLeft, bmp.GetSize()))
{}

SelectionState::SelectionState(const IntRect& r)
  : copy(false),
    floating(false),
    oldRect(IntRect()),
    rect(r)
{}

SelectionState::SelectionState(const Bitmap& bmp, const IntPoint& topLeft,
  const IntRect& oldRect)
  : copy(false),
    floating(true),
    floatingBmp(bmp),
    oldRect(oldRect),
    rect(IntRect(topLeft, bmp.GetSize()))
{}

bool SelectionState::Floating() const{
  return floating;
}

SelectionState& SelectionState::operator=(const SelectionState& state){
  copy = state.copy;
  floating = state.floating;
  floatingBmp = state.floatingBmp;
  oldRect = state.oldRect;
  rect = state.rect;
  return *this;
}

RasterSelection::RasterSelection()
  : m_options(false, Paint(Color(255,255,255)), false)
{}

void RasterSelection::BeginFloat(const Bitmap& src, const copy_selected& copy){
  assert(!Empty());
  if (m_state.floating){
    if (copy.Get()){
      m_state.copy = true;
    }
    return;
  }
  assert(fully_inside(m_state.rect, src));
  m_state.copy = copy.Get();
  m_state.floating = true;
  m_state.oldRect = m_state.rect;
  SetFloatingBitmap(subbitmap(src, m_state.rect), m_state.rect.TopLeft());
}

void RasterSelection::Clip(const IntRect& clipRegion){
  if (m_state.floating || Empty()){
    return;
  }
  m_state.rect = intersection(m_state.rect, clipRegion);
}

bool RasterSelection::Copying() const{
  return m_state.copy;
}

bool RasterSelection::Contains(const IntPoint& pos){
  return Exists() ? m_state.rect.Contains(pos) : false;
}

void RasterSelection::Deselect(){
  m_state.copy = false;
  m_state.floating = false;
  m_state.floatingBmp = Bitmap();
  m_state.oldRect = IntRect();
  m_state.rect = IntRect();
}

void RasterSelection::DrawFloating(FaintDC& dc) const{
  if (Empty() || !m_state.floating){
    return;
  }
  if (!m_state.copy){
    // Clear the source region with the background color
    dc.Rectangle(tri_from_rect(floated(m_state.oldRect)),
      eraser_rectangle_settings(m_options.bg));
  }

  dc.Blit(m_state.floatingBmp, floated(m_state.rect.TopLeft()),
    bitmap_mask_settings(m_options.mask, m_options.bg, m_options.alpha));
}

void RasterSelection::DrawOutline(Overlays& overlays) const{
  if (Empty()){
    return;
  }
  overlays.Rectangle(floated(m_state.rect));
}

bool RasterSelection::Empty() const{
  return empty(m_state.rect);
}

bool RasterSelection::Exists() const{
  return !empty(m_state.rect);
}

bool RasterSelection::Floating() const{
  return m_state.floating;
}

const Bitmap& RasterSelection::GetBitmap() const{
  assert(Floating());
  return m_state.floatingBmp;
}

const Paint& RasterSelection::GetBackground() const{
  return m_options.bg;
}

IntRect RasterSelection::GetRect() const{
  assert(Exists());
  return m_state.rect;
}

IntRect RasterSelection::GetOldRect() const{
  assert(!m_state.copy);
  return m_state.oldRect;
}

SelectionOptions RasterSelection::GetOptions() const{
  return m_options;
}

IntSize RasterSelection::GetSize() const{
  return m_state.rect.GetSize();
}

const SelectionState& RasterSelection::GetState() const{
  return m_state;
}

void RasterSelection::Move(const IntPoint& topLeft){
  m_state.rect.x = topLeft.x;
  m_state.rect.y = topLeft.y;
}

void RasterSelection::OffsetOrigin(const IntPoint& delta){
  m_state.rect.x += delta.x;
  m_state.rect.y += delta.y;
  m_state.oldRect.x += delta.x;
  m_state.oldRect.y += delta.y;
}

void RasterSelection::Paste(const Bitmap& bmp, const IntPoint& topLeft){
  m_state.copy = true; // Don't erase anything
  m_state.floating = true;
  m_state.floatingBmp = bmp;
  m_state.oldRect = IntRect();
  m_state.rect = IntRect(topLeft, bmp.GetSize());
}

void RasterSelection::SetAlphaBlending(bool alpha){
  m_options.alpha = alpha;
}

void RasterSelection::SetBackground(const Paint& bg){
  m_options.bg = bg;
}

void RasterSelection::SetFloatingBitmap(const Bitmap& bmp,
  const IntPoint& topLeft)
{
  assert(m_state.floating);
  m_state.floatingBmp = bmp;
  m_state.rect = IntRect(topLeft, bmp.GetSize());
}

void RasterSelection::SetMask(bool enable){
  m_options.mask = enable;
}

void RasterSelection::SetOptions(const SelectionOptions& options){
  m_options = options;
}

void RasterSelection::SetRect(const IntRect& rect){
  assert(!m_state.floating);
  m_state.copy = false;
  m_state.floating = false;
  m_state.floatingBmp = Bitmap();
  m_state.oldRect = IntRect();
  m_state.rect = rect;
}

void RasterSelection::SetState(const SelectionState& state){
  m_state = state;
}

IntPoint RasterSelection::TopLeft() const{
  assert(!Empty());
  return m_state.rect.TopLeft();
}

SelectionOptions raster_selection_options(const Settings& s){
  return SelectionOptions(
    masked_background(s), s.Get(ts_Bg), alpha_blending(s));
}

void update_mask(bool enableMask, const Paint& bg, bool enableAlpha,
  RasterSelection& selection)
{
  selection.SetMask(enableMask);
  selection.SetBackground(bg);
  selection.SetAlphaBlending(enableAlpha);
}

namespace sel{

Existing::Existing(const RasterSelection& selection)
  : m_selection(selection)
{}

const RasterSelection& Existing::Get() const{
  return m_selection;
}

const SelectionState& Existing::GetState() const{
  return m_selection.GetState();
}

Rectangle::Rectangle(const IntRect& r, const SelectionOptions& options)
  : m_options(options),
    m_rect(r)
{}

IntPoint Rectangle::TopLeft() const{
  return m_rect.TopLeft();
}

IntRect Rectangle::Rect() const{
  return m_rect;
}

const SelectionOptions& Rectangle::GetOptions() const{
  return m_options;
}

Floating::Floating(const IntRect& rect, const Bitmap& bmp,
  const SelectionOptions& options)
  : Rectangle(rect, options),
    m_bitmap(bmp)
{}

const Bitmap& Floating::GetBitmap() const{
  return m_bitmap;
}

Moving::Moving(const IntRect& r, const IntRect& old, const Bitmap& bmp,
  const SelectionOptions& options)
  : Floating(r, bmp, options),
    m_oldRect(old)
{}

IntRect Moving::OldRect() const{
  return m_oldRect;
}

Copying::Copying(const IntRect& r, const Bitmap& bmp,
  const SelectionOptions& options)
  : Floating(r, bmp, options)
{}

}} // namespace
