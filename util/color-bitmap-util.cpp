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
#include "geo/int-rect.hh"
#include "geo/line.hh"
#include "rendering/cairo-context.hh" // For cairo_gradient_bitmap
#include "bitmap/bitmap-templates.hh"
#include "util/color-bitmap-util.hh"

namespace faint{

template<void (*scanline_func)(Bitmap&, int, const Color&)>
void alpha_gradient_bitmap(Bitmap& bmp, const ColRGB& rgb, const int length){
  const double alphaPerLine = 255.0 / length;
  for (int i = 0; i != length; i++){
    Color c(rgb, static_cast<uchar>(i * alphaPerLine));
    scanline_func(bmp, i, c);
  }
}

Bitmap alpha_gradient_bitmap_vertical(const ColRGB& rgb, const IntSize& size){
  return create_bitmap(size,
    [&](Bitmap& bmp){
      alpha_gradient_bitmap<horizontal_scanline>(bmp, rgb, size.h);
    });
}

Bitmap alpha_gradient_bitmap_horizontal(const ColRGB& rgb, const IntSize& size){
  return create_bitmap(size,
    [&](Bitmap& bmp){
      alpha_gradient_bitmap<vertical_scanline>(bmp, rgb, size.w);
    });
}

Bitmap color_bitmap(const Color& color, const IntSize& size,
  const IntSize& patternSize)
{
  const ColRGB c1(blend_alpha(color, grayscale_rgb((192))));
  const ColRGB c2(blend_alpha(color, grayscale_rgb((255))));

  return create_bitmap(size, Color(c1, 255),
    [&](Bitmap& bmp){
      IntSize radius(patternSize / 2);
      for (int y = 0; y < size.h; y += patternSize.h){
        for (int x = 0; x < size.w; x += patternSize.w){
          fill_rect_rgb(bmp, IntRect(IntPoint(x,y), radius), c2);
          fill_rect_rgb(bmp, IntRect(IntPoint(x + radius.w,y + radius.h),
            radius), c2);
        }
      }
    });
}

Bitmap color_bitmap(const Color& color, const IntSize& size){
  return color_bitmap(color, size, size);
}

Bitmap default_pattern(const IntSize& sz){
  Bitmap bmp(sz, color_gray);
  int dx = sz.w / 6;
  int dy = sz.w / 2;
  auto s = solid_1px(color_black);
  draw_hline(bmp, dy, {0,dx}, s);

  draw_line(bmp, {{dx, dy}, {dx * 2, 0}}, s);
  draw_line(bmp, {{dx, dy}, {dx * 2, 2 * dy}}, s);

  draw_hline(bmp, 0, {dx * 2, dx * 4}, s);
  draw_hline(bmp, 2 * dy, {dx * 2, dx * 4}, s);

  draw_line(bmp, {{dx * 4, 0}, {dx * 5, dy}}, s);
  draw_line(bmp, {{dx * 4, 2 * dy}, {dx * 5, dy}}, s);

  draw_hline(bmp, dy, {dx * 5, dx * 6}, s);

  return bmp;
}

Bitmap gradient_bitmap(const Gradient& g, const IntSize& sz){
  Bitmap bmp(cairo_gradient_bitmap(g, sz));
  Bitmap bg(paint_bitmap(Paint(color_transparent_white), sz));
  blend(at_top_left(bmp), onto(bg));
  return bg;
}

Bitmap hue_saturation_color_map(const IntSize& size){
  Bitmap bmp(size);
  const double centerL = 0.5;

  double hueRate = 360.0 / size.w;
  double saturationRate = 1.0 / size.h;
  for (int y = 0; y != size.h; y++){
    for (int x = 0; x != size.w; x++){
      HSL hsl(x * hueRate, 1.0 - y * saturationRate, centerL);
      put_pixel_raw(bmp, x, y, Color(to_rgb(hsl), 255));
    }
  }
  return bmp;
}

Bitmap lightness_gradient_bitmap(const HS& hueSat, const IntSize& size){
  return create_bitmap(size,
    [&](Bitmap& bmp){
      double lightnessRate = 1.0 / bmp.m_h;
      for (int y = 0; y != bmp.m_h; y++){
        horizontal_scanline(bmp, y, to_rgba(HSL(hueSat, y * lightnessRate)));
      }
    });
}

Bitmap with_border(const Bitmap& bmp, LineStyle style){
  return with_border(bmp, color_black, style);
}

Bitmap with_border(const Bitmap& src, const Color& c, LineStyle style){
  return create_bitmap(src,
    [&](Bitmap& bmp){
      draw_rect(bmp, {IntPoint(0,0), bmp.GetSize()}, {c, 1,
        style});
    });
}

static Bitmap pattern_bitmap(const IntSize& sz){
  Bitmap bmp(sz, grayscale_rgba(220));
  const auto s = solid_1px(grayscale_rgba(120));
  int dy = sz.h / 3;
  int dx = sz.w / 4;
  // Across
  draw_hline(bmp, dy, {0, sz.w}, s);
  draw_hline(bmp, dy * 2, {0, sz.w}, s);

  // Down
  draw_vline(bmp, dx * 2, {0, dy}, s);
  draw_vline(bmp, dx, {dy, dy * 2}, s);
  draw_vline(bmp, dx * 3, {dy, dy * 2}, s);
  draw_vline(bmp, dx * 2, {dy * 2, dy * 3}, s);

  return bmp;
}

Bitmap paint_bitmap(const Paint& paint, const IntSize& sz){
  return paint_bitmap(paint, sz, sz);
}

Bitmap paint_bitmap(const Paint& paint,
  const IntSize& sz,
  const IntSize& patternSize)
{
  return visit(paint,
    [&](const Color& c){return color_bitmap(c, sz, patternSize);},
    [&](const Pattern&){return pattern_bitmap(sz);},
    [&](const Gradient& g){return gradient_bitmap(g, sz);});
}

Color discernible_border_color(const Paint& bg){
  return visit(bg,
    [&](const Color& c){
      return ((sum_rgb(c) / 3) < 30 && c.a > 50) ?
        color_white : color_black;
    },
    [&](const Pattern&){
      return color_black;
    },
    [&](const Gradient&){
      return color_black;
    });
}

Bitmap resize_bitmap(const Paint& bgPaint,
  const IntSize& size,
  const Bitmap& illustration,
  const IntPoint& pos)
{
  IntSize patternSize(size / 2);
  auto bgBmp(with_border(paint_bitmap(bgPaint, size, patternSize)));
  blit(offsat(illustration, pos), onto(bgBmp));
  return bgBmp;
}

} // namespace
