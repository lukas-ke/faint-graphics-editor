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

#include <algorithm>
#include "wx/dcbuffer.h"
#include "geo/geo-func.hh"
#include "geo/int-point.hh"
#include "geo/int-rect.hh"
#include "geo/int-size.hh"
#include "gui/bitmap-list-ctrl.hh"
#include "util/index-iter.hh"
#include "util/status-interface.hh"
#include "util-wx/bind-event.hh"
#include "util-wx/fwd-bind.hh"
#include "util-wx/gui-util.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/fwd-wx.hh"

namespace faint{

const wxEventType FAINT_BITMAPLIST_SELECTION = wxNewEventType();

const wxEventTypeTag<wxCommandEvent> EVT_FAINT_BITMAPLIST_SELECTION(
  FAINT_BITMAPLIST_SELECTION);

static IntSize cell_size(const IntSize& imageSize, const IntSize& spacing){
  return imageSize + 2 * spacing;
}

static IntRect selection_rect(const IntPoint& delta,
  const Index& selection,
  const IntSize& imageSize,
  const IntSize& spacing)
{
  const IntSize cellSize(cell_size(imageSize, spacing));
  const IntPoint topLeft(delta * selection.Get() * point_from_size(cellSize));
  return {topLeft, cellSize};
}

static void fill_selection_rect(wxDC& dc, const IntRect& r){
  wxPen pen(wxColour(0, 0, 0));
  pen.SetStyle(wxPENSTYLE_TRANSPARENT);
  dc.SetPen(pen);
  dc.SetBrush(wxBrush(get_gui_selected_color()));
  dc.DrawRectangle(to_wx(r));
}

static IntPoint bmp_top_left(const Index& i, const IntPoint& delta,
  const IntSize& imageSize,
  const IntSize& spacing)
{
  return delta * i.Get() * point_from_size(cell_size(imageSize, spacing)) +
    point_from_size(spacing);
}

static void paint_list_ctrl(wxDC& dc,
  const IntSize& spacing,
  const IntSize& imageSize,
  const Index& selection,
  const std::vector<wxBitmap>& images,
  Axis orientation)
{
  // Control the offset direction when drawing the bitmaps to layout
  // horizontally or vertically.
  const auto delta = orientation == Axis::HORIZONTAL ?
    IntPoint(1, 0) :
    IntPoint(0,1);

  dc.SetBackground(get_gui_deselected_color());
  dc.Clear();

  fill_selection_rect(dc, selection_rect(delta, selection, imageSize, spacing));

  for (auto i : up_to(images.size())){
    const wxBitmap& bmp(images[i.Get()]);
    dc.DrawBitmap(bmp,
      to_wx(bmp_top_left(i, delta, imageSize, spacing)));
  }
}

static Index get_index_at(const IntPoint& pos,
  size_t numImages,
  Axis orientation,
  const IntSize& imageSize,
  const IntSize& spacing)
{
  assert(numImages != 0);
  const int maxIndex = resigned(numImages) - 1;
  IntSize cellSize = cell_size(imageSize, spacing);
  IntPoint indexes = pos / point_from_size(cellSize);
  return Index(std::min(orientation == Axis::HORIZONTAL ?
    indexes.x : indexes.y, maxIndex));
}

BitmapListCtrl::BitmapListCtrl(wxWindow* parent,
  const IntSize& imageSize,
  StatusInterface& status,
  Axis orientation,
  IntSize spacing)
  : wxPanel(parent),
    m_imageSize(imageSize),
    m_orientation(orientation),
    m_selection(0),
    m_spacing(spacing),
    m_status(status)
{
  SetBackgroundStyle(wxBG_STYLE_PAINT);
  set_size(this, cell_size(m_imageSize, m_spacing));

  events::on_paint(this, [this](){
    wxAutoBufferedPaintDC paintDC(this);
    paint_list_ctrl(paintDC, m_spacing,
      m_imageSize,
      m_selection,
      m_images,
      m_orientation);
  });

  events::on_mouse_left_down(this,
    [this](const IntPoint& pos){
      auto selected = get_index_at(pos,
        m_images.size(),
        m_orientation,
        m_imageSize,
        m_spacing);

      if (selected != m_selection){
        SetSelection(selected);
        wxCommandEvent newEvent(FAINT_BITMAPLIST_SELECTION, GetId());
        newEvent.SetEventObject(this);
        GetEventHandler()->ProcessEvent(newEvent);
      }
    });

  events::on_mouse_leave_window(this, [this](){
    m_status.SetMainText("");
  });

  events::on_mouse_motion(this,
    [this](const IntPoint& pos){
      auto index = get_index_at(pos,
        m_images.size(),
        m_orientation,
        m_imageSize,
        m_spacing);
      m_status.SetMainText(m_statusTexts[to_size_t(index)]);
    });
}

Index BitmapListCtrl::Add(const wxBitmap& bmp, const utf8_string& statusText){
  assert(bmp.GetWidth() == m_imageSize.w);
  assert(bmp.GetHeight() == m_imageSize.h);

  m_images.push_back(bmp);
  m_statusTexts.push_back(statusText);
  const int numImages = resigned(m_images.size());

  auto scale = m_orientation == Axis::VERTICAL ?
    IntSize(1, numImages) :
    IntSize(numImages, 1);
  set_size(this, cell_size(m_imageSize, m_spacing) * scale);

  return Index(numImages - 1);
}

Index BitmapListCtrl::GetSelection() const {
  return m_selection;
}

void BitmapListCtrl::SetSelection(const Index& selection){
  assert(valid_index(selection, m_images));
  m_selection = selection;
  Refresh();
}

} // namespace

namespace faint{ namespace events{

void on_selection(BitmapListCtrl* ctrl,
  const std::function<void(const Index&)>& f)
{
  bind(ctrl, EVT_FAINT_BITMAPLIST_SELECTION,
    [ctrl, f](){
      f(ctrl->GetSelection());
    });
}

}} // namespace
