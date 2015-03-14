// -*- coding: us-ascii-unix -*-
// Copyright 2013 Lukas Kemmer
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
#include "bitmap/color.hh"
#include "bitmap/draw.hh"
#include "geo/geo-func.hh"
#include "geo/int-rect.hh"
#include "geo/int-size.hh"
#include "geo/line.hh"
#include "geo/range.hh"
#include "gui/slider-common.hh"
#include "util-wx/system-colors.hh"

namespace faint{

const int PADDING = 4;

void draw_styled_slider_bg_rect(Bitmap& bmp, const IntSize& size){
  clear(bmp, color_button_face());
  IntRect r(IntPoint(0, PADDING), size - IntSize(0, PADDING * 2));
  fill_rect_color(bmp, r, color_system_light_3d());
  draw_sunken_ui_border(bmp, r);
}

void SliderRectangleBackground::Draw(Bitmap& bmp, const IntSize& size,
  SliderDir)
{
  draw_styled_slider_bg_rect(bmp, size);
}

SliderBackground* SliderRectangleBackground::Clone() const{
  return new SliderRectangleBackground(*this);
}

static IntLineSegment get_mid_line(const IntSize& size, SliderDir dir){
  if (dir == SliderDir::HORIZONTAL){
    int x = size.w / 2;
    int y0 = size.h - PADDING;
    return {IntPoint(x, y0), IntPoint(x, size.h)};
  }
  else {
    int y = size.h / 2;
    return {IntPoint(0, y), IntPoint(size.w, y)};
  }
}

void SliderMidPointBackground::Draw(Bitmap& bmp, const IntSize& size,
  SliderDir dir)
{
  // draw_slider_bg_rect(bmp, size);
  draw_styled_slider_bg_rect(bmp, size);
  draw_line(bmp, get_mid_line(size, dir),
    {color_inactive_caption(), 3, LineStyle::SOLID, LineCap::BUTT});
}

SliderBackground* SliderMidPointBackground::Clone() const{
  return new SliderMidPointBackground(*this);
}

double pos_to_value(const int pos,
  const int length,
  const ClosedIntRange& range)
{
  const int border = 2;
  double pixels_per_value = (length - border) / double(range.Delta());
  double value = pos / pixels_per_value + range.Lower();
  return value;
}

int value_to_pos(const double value,
  const int length, const
  ClosedIntRange& range)
{
  const int border = 2;
  double pixelsPerValue = (length - border) / double(range.Delta());
  return std::min(rounded(pixelsPerValue * (value - range.Lower()) + border / 2),
    length - border);
}

} // namespace
