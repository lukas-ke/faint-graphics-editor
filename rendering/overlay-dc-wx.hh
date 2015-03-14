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

#ifndef FAINT_OVERLAY_DC_WX_HH
#define FAINT_OVERLAY_DC_WX_HH
#include "geo/int-size.hh"
#include "geo/rect.hh"
#include "rendering/overlay.hh"

class wxDC;

namespace faint{
class TextBuffer;

class OverlayDC_WX : public OverlayDC{
  // An OverlayDC which uses a wxDC to draw the overlays.
public:
  // Constructor arguments:
  // - scale specifies a scaling to apply to coordinates (e.g. a zoom
  // level). Only the placement, spacing and length of the overlays
  // are scaled - line thickness, handle sizes and such remain
  // constant.
  //
  // - visibleRect specifies the visible region of the image, and is
  // used to limit the extents of e.g. HorizontalLine and VerticalLine
  // which do not need to extend past the visible area.
  //
  // - imageSize is the actual size of the backing bitmap.
  OverlayDC_WX(wxDC&,
    coord scale,
    const Rect& visibleRect,
    const IntSize& imageSize,
    int objectHandleWidth);

  void Caret(const LineSegment&) override;
  void ConstrainPos(const Point&) override;
  void Corners(const Tri&) override;
  void ExtensionPoint(const Point&) override;
  void GridLines(const Grid&, const IntPoint&, const IntSize&) override;
  void Handles(const Rect&) override;
  void Handles(const Tri&) override;
  void HorizontalLine(coord) override;
  void Line(const LineSegment&) override;
  void MovablePoint(const Point&) override;
  void Parallelogram(const Tri&) override;
  void Pivot(const Point&) override;
  void Pixel(const IntPoint&) override;
  void Rectangle(const Rect&) override;
  void Textbox(const Point&, const TextBuffer&, const utf8_string&) override;
  void Text(const Point&, const utf8_string&) override;
  void VerticalLine(coord) override;

  OverlayDC_WX(const OverlayDC_WX&) = delete;
  OverlayDC_WX& operator=(const OverlayDC_WX&) = delete;
private:
  wxDC& m_dc;
  Rect m_imageRect;
  IntSize m_imageSize;
  int m_objectHandleWidth;
  coord m_scale;
};

} // namespace

#endif
