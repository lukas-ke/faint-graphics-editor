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

#include <cassert>
#include <cmath>
#include "bitmap/auto-crop.hh"
#include "bitmap/draw.hh"
#include "bitmap/scale-bilinear.hh"
#include "commands/command.hh"
#include "commands/set-bitmap-cmd.hh"
#include "geo/axis.hh"
#include "geo/axis.hh"
#include "geo/geo-func.hh"
#include "geo/int-rect.hh"
#include "geo/pathpt.hh"
#include "geo/scale.hh"
#include "objects/objraster.hh"
#include "rendering/cairo-context.hh" // For cairo_skew
#include "rendering/faint-dc.hh"
#include "text/utf8-string.hh"
#include "util/at-most.hh"
#include "util/default-settings.hh"
#include "util/object-util.hh"
#include "util/type-util.hh"

namespace faint{

static void apply_transform(const Bitmap& src,
  const Tri& transform,
  Bitmap& dst)
{
  dst = src;
  Tri t2(transform);
  coord skew = t2.Skew();
  Angle angle = t2.GetAngle();
  if (!rather_zero(angle)){
    t2 = rotated(t2, -angle, t2.P0());
  }
  if (!rather_zero(skew)){
    t2 = skewed(t2, - skew);
    coord skew_pixels = skew / (t2.Width() / src.m_w);
    Angle skew_angle = atan2(skew_pixels,
      (t2.P0().y - t2.P2().y) / (t2.Height() / src.m_h));
    dst = cairo_skew(dst, tan(skew_angle), skew_pixels);
  }

  if (!rather_zero(angle)){
    t2 = rotated(t2, angle, t2.P0());
  }

  if (!coord_eq(fabs(t2.Width()), src.m_w) ||
    !coord_eq(fabs(t2.Height()), src.m_h))
  {
    // Only scale bilinear if the width is truly different, not just
    // inverted.
    dst = scale_bilinear(dst, rounded(Size(std::fabs(t2.Width()) + 1,
      std::fabs(t2.Height()) + 1)));
  }

  if (t2.Width() < 0){
    dst = flip(dst, along(Axis::HORIZONTAL));
  }
  if (t2.Height() < 0){
    dst = flip(dst, across(Axis::HORIZONTAL));
  }

  if (!rather_zero(angle)){
    dst = rotate_bilinear(dst, angle);
  }
}

ObjRaster::ObjRaster(const Tri& tri, const Bitmap& bitmap, const Settings& s)
  : StandardObject(s),
    m_bitmap(bitmap),
    m_scaled(bitmap),
    m_tri(tri)
{
  assert(m_settings.Has(ts_BackgroundStyle));
  assert(m_settings.Has(ts_Bg));
  apply_transform(m_bitmap, m_tri, m_scaled);
}

ObjRaster::ObjRaster(const ObjRaster& other)
  : StandardObject(other.GetSettings()),
    m_bitmap(other.m_bitmap),
    m_scaled(other.m_scaled),
    m_tri(other.GetTri())
{}

void ObjRaster::Draw(FaintDC& dc, ExpressionContext&){
  Draw(dc);
}

void ObjRaster::Draw(FaintDC& dc){
  Rect r = bounding_rect(m_tri);
  // Shift the bitmap half a pixel so that handles anchor inside like
  // for other objects
  // Fixme #1: Should this be done inside FaintDC instead?
  // Fixme #2: Didn't I remove realigning of other objects?)
  Point shift(-0.5, -0.5);
  dc.Blit(m_scaled, r.TopLeft() + shift, m_settings);
}

void ObjRaster::DrawMask(FaintDC& dc, ExpressionContext&){
  // Fixme: This draws a filled axis aligned rectangle.  Preferably,
  // the object's masked color (if any) should be excluded, and the area
  // aligned with the object (As for alignment, at the time only the
  // non-aligned rectangle function is blended)
  Settings s(default_rectangle_settings());
  s.Set(ts_FillStyle, FillStyle::FILL);
  s.Set(ts_Fg, Paint(mask_fill));
  s.Set(ts_AntiAlias, 0);
  dc.Rectangle(m_tri, s);
}

coord ObjRaster::GetArea() const{
  return area(m_tri);
}

std::vector<Point> ObjRaster::GetAttachPoints() const{
  return get_attach_points(m_tri);
}

std::vector<PathPt> ObjRaster::GetPath(const ExpressionContext&) const{
  return {
    PathPt::MoveTo(m_tri.P0()),
    PathPt::LineTo(m_tri.P1()),
    PathPt::LineTo(m_tri.P3()),
    PathPt::LineTo(m_tri.P2()),
    PathPt::PathCloser()
  };
}

IntRect ObjRaster::GetRefreshRect() const{
  return floored(inflated(bounding_rect(m_tri), 2.0));
}

Tri ObjRaster::GetTri() const{
  return m_tri;
}

utf8_string ObjRaster::GetType() const{
  return "Raster";
}

Object* ObjRaster::Clone() const{
  return new ObjRaster(*this);
}

bool ObjRaster::HitTest(const Point& p){
  return bounding_rect(m_tri).Contains(p);
}

Bitmap& ObjRaster::GetBitmap(){
  return m_bitmap;
}

const Bitmap& ObjRaster::GetBitmap() const{
  return m_bitmap;
}

void ObjRaster::SetBitmap(const Bitmap& bmp){
  m_bitmap = bmp;
  apply_transform(m_bitmap, m_tri, m_scaled);
}

void ObjRaster::SetTri(const Tri& t){
  m_tri = t;
  apply_transform(m_bitmap, t, m_scaled);
}

static CommandPtr crop_to_rect(const IntRect& r,
  ObjRaster* obj,
  const Bitmap& bmp)
{
  coord old_w = static_cast<coord>(bmp.m_w);
  coord old_h = static_cast<coord>(bmp.m_h);
  coord new_w = static_cast<coord>(r.w);
  coord new_h = static_cast<coord>(r.h);
  coord new_x = static_cast<coord>(r.x);
  coord new_y = static_cast<coord>(r.y);

  Tri t = obj->GetTri();
  t = scaled(t, Scale(new_w / old_w, new_h / old_h), t.P0());
  t = offset_aligned(t, new_x, new_y);

  return set_object_bitmap_command(obj,
    subbitmap(bmp, r),
    t, "Crop Raster Object");
}

CommandPtr crop_raster_object_command(ObjRaster* obj){
  const Bitmap& bmp(obj->GetBitmap());
  return get_auto_crop_rectangles(bmp).Visit(
    []() -> CommandPtr {
      return nullptr;
    },
    [&bmp, &obj](const IntRect& r){
      return crop_to_rect(r, obj, bmp);
    },
    [&bmp, &obj](const IntRect& r0, const IntRect&){
      // Fixme: DWIM with second rect. However: Rework DWIM first, add
      // a generic DWIM-command composing the commands, rather than
      // adding DWIM to set_object_bitmap_command.
      return crop_to_rect(r0, obj, bmp);
    });
}

Tri tri_for_bmp(const Point& p0, const Bitmap& bmp){
  Size sz(floated(bmp.GetSize()));
  return Tri(p0, p0 + delta_x(sz.w -1.0), p0 + delta_y(sz.h - 1.0));
}

bool is_raster(const Object& obj){
  return is_type<ObjRaster>(obj);
}


ObjRaster* create_raster_object_raw(const Tri& tri,
  const Bitmap& bmp,
  const Settings& settings)
{
  return new ObjRaster(tri, bmp, settings);

}

ObjectPtr create_raster_object(const Tri& tri,
  const Bitmap& bmp,
  const Settings& settings)
{
  return std::make_unique<ObjRaster>(tri, bmp, settings);
}

} // namespace
