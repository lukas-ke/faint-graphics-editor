// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
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

#include "bitmap/draw.hh"
#include "bitmap/pattern.hh"
#include "geo/int-point.hh"
#include "geo/int-rect.hh"
#include "gui/slider-alpha-background.hh"
#include "util/color-bitmap-util.hh"

namespace faint{

Bitmap alpha_background_bitmap(const ColRGB& rgb,
  const IntSize& size,
  SliderDir dir)
{
  if (dir == SliderDir::HORIZONTAL){
    return alpha_gradient_bitmap_horizontal(rgb, size);
  }
  else{
    return alpha_gradient_bitmap_vertical(rgb, size);
  }
}

AlphaBackground::AlphaBackground(const ColRGB& rgb)
  : m_rgb(rgb)
{
  const int cellW = 10;
  Bitmap bg(IntSize(cellW*2,cellW*2), color_white);
  const Color alpha_gray(grayscale_rgba(192));
  // Checkered background
  fill_rect_color(bg, IntRect(IntPoint(cellW,0), IntSize(cellW,cellW)),
    alpha_gray);

  fill_rect_color(bg, IntRect(IntPoint(0,cellW), IntSize(cellW,cellW)),
    alpha_gray);
  m_bgPattern = Paint(Pattern(bg));
}

void AlphaBackground::Draw(Bitmap& bmp, const IntSize& size, SliderDir dir){
  if (!bitmap_ok(m_bitmap) || m_bitmap.GetSize() != size){
    m_bitmap = alpha_background_bitmap(m_rgb, size, dir);
  }

  clear(bmp, Color(255,255,255));
  fill_rect(bmp, IntRect(IntPoint(0,0), bmp.GetSize()), m_bgPattern);
  blend(at_top_left(m_bitmap), onto(bmp));
}

SliderBackground* AlphaBackground::Clone() const{
  return new AlphaBackground(*this);
}

} // namespace
