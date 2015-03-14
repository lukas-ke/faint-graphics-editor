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
#include "wx/dc.h"
#include "wx/settings.h"
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/draw.hh"
#include "geo/geo-func.hh"
#include "geo/line.hh"
#include "geo/tri.hh"
#include "rendering/overlay-dc-wx.hh"
#include "text/text-buffer.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/font.hh"
#include "util-wx/fwd-dc-wx.hh"
#include "util-wx/system-colors.hh"
#include "util/grid.hh"

namespace faint{

const int g_movableRadius = 2;

static void draw_wx_bitmap(wxDC& dc, const Bitmap& bmp, const IntPoint& topLeft){
  dc.DrawBitmap(to_wx_bmp(bmp), topLeft.x, topLeft.y);
}

static void draw_corner(wxDC& dc,
  const Angle& angle,
  const Point& pt,
  bool left,
  bool top)
{
  coord len(10.0);
  Point delta((left ? len : -len) * cos(left ? angle : -angle),
    len * sin(left ? angle : -angle));

  draw_line(dc, rounded(pt), rounded(pt + delta));
  if ((left && top) || !(left || top)){
    delta.y *= -1;
  }
  if ((!left && top) || (left && !top)){
    delta.x *= - 1;
  }
  draw_line(dc, rounded(pt), rounded(pt + transposed(delta)));
}

static void draw_handle(wxDC& dc, const Point& p, coord scale, int width){
  dc.DrawRectangle(truncated(p.x * scale - width / 2.0 + 0.5),
    truncated(p.y * scale - width / 2.0 + 0.5), width, width);
}

static Bitmap get_grid_bitmap(const Grid& grid,
  const IntPoint& imageRegionTopLeft,
  double scale,
  const IntSize& size)
{
  Bitmap bmp(size + IntSize(5,5), Color(0,0,0,0));
  int spacing = rounded(grid.Spacing() * scale);
  const Color color(grid.GetColor());
  LineStyle style = grid.Dashed() && spacing >= 8 ? LineStyle::LONG_DASH :
    LineStyle::SOLID;
  Point anchor = grid.Anchor();
  int xOff = truncated(imageRegionTopLeft.x * scale - anchor.x * scale) % spacing;
  int yOff = truncated(imageRegionTopLeft.y * scale - anchor.y * scale) % spacing;
  int maxPos = std::max((size.w + 10) / grid.Spacing(),
    (size.h + 10) / grid.Spacing()) + 2;
  const int width = 1;

  const LineSettings s(color, width, style, LineCap::BUTT);
  for (int i = 0; i != maxPos; i++){
    coord x = i * spacing - xOff;
    draw_vline(bmp, floored(x), {0, bmp.m_h}, s);

    coord y = i * spacing - yOff;
    draw_hline(bmp, floored(y), {0, bmp.m_w}, s);
  }

  return bmp;
}

static void overlay_pen_and_brush(wxDC& dc){
  set_pen(dc, color_black);
  set_brush(dc, Color(128, 128, 200));
}

static void overlay_transparent_brush(wxDC& dc){
  set_transparent_brush(dc);
  set_pen(dc, color_black);
}

OverlayDC_WX::OverlayDC_WX(wxDC& dc,
  coord scale,
  const Rect& imageRect,
  const IntSize& imageSize,
  int objectHandleWidth)
  : m_dc(dc),
    m_imageRect(imageRect),
    m_imageSize(imageSize),
    m_objectHandleWidth(objectHandleWidth),
    m_scale(scale)
{}

void OverlayDC_WX::Caret(const LineSegment& line){
  wxRasterOperationMode oldMode(m_dc.GetLogicalFunction());
  m_dc.SetLogicalFunction(wxINVERT);
  set_pen(m_dc, color_black);
  draw_line(m_dc, floored(line.p0 * m_scale), floored(line.p1 * m_scale));
  m_dc.SetLogicalFunction(oldMode);
}

void OverlayDC_WX::ConstrainPos(const Point& pos){
  overlay_transparent_brush(m_dc);
  const int constrainRadius = 2;
  m_dc.DrawCircle(rounded(pos.x * m_scale), rounded(pos.y * m_scale),
    constrainRadius);
}

void OverlayDC_WX::Corners(const Tri& tri){
  set_dotted_pen(m_dc, color_magenta);
  set_transparent_brush(m_dc);

  Angle angle = tri.GetAngle();
  Point topLeft = tri.P0() * m_scale;
  Point topRight = tri.P1() * m_scale;
  Point bottomLeft = tri.P2() * m_scale;
  Point bottomRight = tri.P3() * m_scale;
  draw_corner(m_dc, angle, topLeft, true, true);
  draw_corner(m_dc, angle, topRight, false, true);
  draw_corner(m_dc, angle, bottomRight, false, false);
  draw_corner(m_dc, angle, bottomLeft, true, false);
  set_pen(m_dc, color_black);
}

void OverlayDC_WX::ExtensionPoint(const Point& center){
  set_pen(m_dc, color_black);
  set_brush(m_dc, color_white);
  m_dc.DrawCircle(rounded(center.x * m_scale), rounded(center.y * m_scale),
    g_movableRadius);
}

static bool grid_useless_at_scale(const Grid& g, coord scale){
  return coord(g.Spacing()) * scale < 2.0;
}

void OverlayDC_WX::GridLines(const Grid& grid,
  const IntPoint& imageRegionTopLeft,
  const IntSize& size)
{
  if (grid_useless_at_scale(grid, m_scale)){
    return;
  }

  Bitmap bmp(get_grid_bitmap(grid, imageRegionTopLeft, m_scale, size));

  // + 1 so that a grid-line on the far edge is visible (wxRect does
  // not include far edge)
  m_dc.SetClippingRegion(wxRect(0, 0,
      truncated(m_imageSize.w * m_scale + 1.0),
      truncated(m_imageSize.h * m_scale + 1.0)));
  draw_wx_bitmap(m_dc, bmp, floored(imageRegionTopLeft * m_scale));
  m_dc.DestroyClippingRegion();
}

void OverlayDC_WX::Handles(const Rect& rect){
  overlay_pen_and_brush(m_dc);
  draw_handle(m_dc, rect.TopLeft(), m_scale, m_objectHandleWidth);
  draw_handle(m_dc, rect.TopRight(), m_scale, m_objectHandleWidth);
  draw_handle(m_dc, rect.BottomLeft(), m_scale, m_objectHandleWidth);
  draw_handle(m_dc, rect.BottomRight(), m_scale, m_objectHandleWidth);
}

void OverlayDC_WX::Handles(const Tri& tri){
  overlay_pen_and_brush(m_dc);
  draw_handle(m_dc, tri.P0(), m_scale, m_objectHandleWidth);
  draw_handle(m_dc, tri.P1(), m_scale, m_objectHandleWidth);
  draw_handle(m_dc, tri.P2(), m_scale, m_objectHandleWidth);
  draw_handle(m_dc, tri.P3(), m_scale, m_objectHandleWidth);
}

void OverlayDC_WX::HorizontalLine(coord in_y){
  set_dotted_pen(m_dc, color_magenta);
  int y = truncated(in_y * m_scale);
  int x0 = floored(m_imageRect.x * m_scale);
  int x1 = floored((m_imageRect.x + m_imageRect.w) * m_scale);
  m_dc.DrawLine(x0, y, x1, y);
}

void OverlayDC_WX::Line(const LineSegment& line){
  set_pen(m_dc, color_magenta);
  IntPoint p0 = floored(line.p0 * m_scale);
  IntPoint p1 = floored(line.p1 * m_scale);
  m_dc.DrawLine(p0.x, p0.y, p1.x, p1.y);
}

void OverlayDC_WX::MovablePoint(const Point& center){
  overlay_pen_and_brush(m_dc);
  m_dc.DrawCircle(rounded(center.x * m_scale), rounded(center.y * m_scale),
    g_movableRadius);
}

void OverlayDC_WX::Parallelogram(const Tri& tri){
  set_dotted_pen(m_dc, color_magenta);
  set_transparent_brush(m_dc);
  draw_polygon(m_dc, {
    rounded(tri.P0()),
    rounded(tri.P1()),
    rounded(tri.P3()),
    rounded(tri.P2())});
  set_pen(m_dc, color_black);
}

void OverlayDC_WX::Pivot(const Point& center){
  overlay_pen_and_brush(m_dc);
  const int pivotRadius = 5;
  m_dc.DrawCircle(rounded(center.x * m_scale), rounded(center.y * m_scale),
    pivotRadius);
}

void OverlayDC_WX::Pixel(const IntPoint& imagePoint){
  set_pen(m_dc, color_magenta);
  Point p = imagePoint * m_scale;

  // Negative and positive offsets, wxWidgets excludes the
  // end-coordinate of lines.
  coord dn = 5.0;
  coord dp = 6.0;

  if (m_scale <= 1){
    // Draw a simple crosshair for 1:1 and less
    draw_line(m_dc, floored(p - delta_y(dn)), floored(p + delta_y(dp)));
    draw_line(m_dc, floored(p - delta_x(dn)), floored(p + delta_x(dp)));
  }
  else {
    // Draw a "hash"-box around the pixel
    draw_line(m_dc, floored(p - delta_y(dn)),
      floored(p + delta_y(m_scale + dp))); // Left

    draw_line(m_dc, floored(p - delta_x(dn)), floored(p +
        delta_x(m_scale + dp))); // Top

    draw_line(m_dc, floored(p + delta_xy(-dn, m_scale)),
      floored(p + delta_xy(m_scale + dp, m_scale))); // Bottom

    draw_line(m_dc, floored(p + delta_xy(m_scale, -dn)),
      floored(p + delta_xy(m_scale, m_scale + dp))); // Right
  }
}

void OverlayDC_WX::Rectangle(const Rect& r){
  set_dotted_pen(m_dc, color_magenta);
  set_transparent_brush(m_dc);
  m_dc.DrawRectangle(rounded(r.x * m_scale), rounded(r.y * m_scale),
    rounded(r.w * m_scale), rounded(r.h * m_scale));
  set_pen(m_dc, color_black);
}

void OverlayDC_WX::Textbox(const Point& topLeft, const TextBuffer& text,
  const utf8_string& sampleText)
{
  set_pen(m_dc, color_windowframe());
  // set_brush(m_dc, Color(245, 228, 156));
  set_brush(m_dc, color_window());
  wxFont font(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
  if (text.empty()){
    font = font.Italic();
  }
  m_dc.SetFont(font);

  IntPoint p(rounded(topLeft * m_scale));
  auto textWx(to_wx(text.get()));
  auto sampleWx(to_wx(sampleText));
  auto textSize(m_dc.GetTextExtent(textWx));
  auto sampleSize(m_dc.GetTextExtent(sampleWx));
  auto fontSize(font.GetPixelSize());

  wxArrayInt widths;
  bool ok = m_dc.GetPartialTextExtents(textWx, widths);
  assert(ok);

  int caretPos = text.caret() == 0 ?
    0 :
    widths[text.caret() - 1];

  const int r = 5; // Rounded corner radius
  const int minWidth = std::max(sampleSize.GetWidth() + r * 2, 50);
  const int boxWidth = std::max(textSize.GetWidth() + r * 2, minWidth);
  const int boxHeight = fontSize.GetHeight() + r * 2;

  // Shadow behind the rounded rectangle
  set_pen(m_dc, color_dark_shadow_3d());
  set_brush(m_dc, color_dark_shadow_3d());
  m_dc.DrawRoundedRectangle(p.x + 1, p.y + 1, boxWidth, boxHeight, r);

  // The rounded rectangle
  set_pen(m_dc, color_windowframe());
  set_brush(m_dc, color_button_face());
  m_dc.DrawRoundedRectangle(p.x, p.y, boxWidth, boxHeight, r);

  CaretRange sel = text.get_sel_range();
  if (sel.to != 0){
    set_brush(m_dc, get_highlight_color());
    int s0x = sel.from == 0 ?
      0 : widths[sel.from - 1];

    int s1x = sel.to == 0 ?
      0 : widths[sel.to - 1];

    m_dc.DrawRectangle(p.x + r + s0x, p.y + r, s1x - s0x,
      fontSize.GetHeight());
  }

  auto y0 = p.y + (boxHeight - fontSize.GetHeight()) / 2 - 1;

  if (text.empty()) {
    // Draw the sample text when no text is entered.
    m_dc.SetTextForeground(to_wx(color_graytext()));
    m_dc.DrawText(sampleWx, p.x + r, y0);
  }
  else {
    m_dc.SetTextForeground(to_wx(color_captiontext()));
    m_dc.DrawText(textWx, p.x + r, y0);
  }

  // Draw the caret
  m_dc.DrawLine(p.x + r + caretPos,
    y0,
    p.x + r + caretPos,
    y0 + fontSize.GetHeight());
}

void OverlayDC_WX::Text(const Point& topLeft, const utf8_string& text){
  m_dc.SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
  m_dc.SetTextForeground(wxColour(255, 0, 255));
  IntPoint pixelPos(truncated(topLeft * m_scale));
  m_dc.DrawText(to_wx(text), pixelPos.x, pixelPos.y);
}

void OverlayDC_WX::VerticalLine(coord in_x){
  set_dotted_pen(m_dc, color_magenta);
  int x = truncated(m_scale * in_x);
  int y0 = truncated(m_scale * m_imageRect.y);
  int y1 = truncated(m_scale * (m_imageRect.y + m_imageRect.h));
  m_dc.DrawLine(x, y0, x, y1);
}

} // namespace
