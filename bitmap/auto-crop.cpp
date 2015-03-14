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
#include "bitmap/auto-crop.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/color-counting.hh"
#include "bitmap/color.hh"
#include "bitmap/draw.hh"
#include "geo/geo-func.hh"
#include "geo/int-rect.hh"
#include "util/at-most.hh"
#include "util/optional.hh"

namespace faint{

static IntRect get_auto_crop_rect(const Bitmap& bmp, const Color& bgCol){
  const int width = bmp.m_w;
  const int height = bmp.m_h;
  int y0 = 0;
  int y1 = 0;
  bool upDone = false;
  bool dnDone = false;

  for (int y = 0; y != height; y++){
    for (int x = 0; x!= width; x++){
      if (!upDone){
        if (get_color_raw(bmp, x, y) != bgCol){
          y0 = y;
          upDone = true;
          if (dnDone){
            goto yDone;
          }
        }
      }
      if (!dnDone){
        if (get_color_raw(bmp, x, height - y - 1) != bgCol){
          y1 = height - y;
          dnDone = true;
          if (upDone){
            goto yDone;
          }
        }
      }
    }
  }

  // Post-y computation-label
 yDone:
  int x0 = 0;
  int x1 = 0;
  bool leftDone = false;
  bool rightDone = false;

  for (int x = 0; x!= width; x++){
    for (int y = 0; y!= height; y++){
      if (!leftDone){
        if (get_color_raw(bmp, x, y) != bgCol){
          x0 = x;
          leftDone = true;
          if (rightDone){
            goto xDone;
          }
        }
      }
      if (!rightDone){
        if (get_color_raw(bmp, width - x - 1, y) != bgCol){
          x1 = width - x;
          rightDone = true;
          if (leftDone){
            goto xDone;
          }
        }
      }
    }
  }

  // Done
 xDone:
  return IntRect(IntPoint(x0, y0), IntSize(x1 - x0, y1 - y0));
}

Optional<Color> get_edge_color(const Bitmap& bmp, const IntRect& r){
  if (r.x < 0 || r.x + r.w > bmp.m_w || r.y < 0 || r.y + r.h > bmp.m_h){
    // Invalid rectangle
    return {};
  }

  Color color = get_color_raw(bmp, r.x, r.y);
  for (int x = r.x; x != r.x + r.w; x++){
    if (get_color_raw(bmp, x, r.y) != color ||
      get_color_raw(bmp, x, r.y + r.h - 1) != color)
    {
      return {};
    }
  }

  for (int y = r.y; y != r.y + r.h; y++){
    if (get_color_raw(bmp, r.x + r.w - 1, y) != color ||
      get_color_raw(bmp, r.x, y) != color)
    {
      return {};
    }
  }

  // Color determined
  return option(color);
}

static Bitmap copy_right_column(const Bitmap& bmp){
  return subbitmap(bmp, IntRect(IntPoint(int(bmp.m_w) - 1, 0),
    IntSize(1, int(bmp.m_h))));
}

static Bitmap copy_left_column(const Bitmap& bmp){
  return subbitmap(bmp, IntRect(IntPoint(0, 0), IntSize(1, bmp.m_h)));
}

static Bitmap copy_top_row(const Bitmap& bmp){
  return subbitmap(bmp, IntRect(IntPoint(0, 0), IntSize(bmp.m_w, 1)));
}

static Bitmap copy_bottom_row(const Bitmap& bmp){
  return subbitmap(bmp, IntRect(IntPoint(0, int(bmp.m_h) - 1),
    IntSize(int(bmp.m_w), 1)));
}

Optional<Color> resize_dwim_color(const Bitmap& bmp, const IntRect& r){
  color_counts_t colors;
  if (overextends_right(r, bmp)){
    add_color_counts(copy_right_column(bmp), colors);
  }
  if (overextends_left(r, bmp)){
    add_color_counts(copy_left_column(bmp), colors);
  }
  if (overextends_down(r, bmp)){
    add_color_counts(copy_bottom_row(bmp), colors);
  }
  if (overextends_up(r, bmp)){
    add_color_counts(copy_top_row(bmp), colors);
  }
  if (colors.empty()){
    // No edge colors retrieved if shrinking or unchanged.
    return {};
  }
  return option(most_common(colors));
}

Color most_common_edge_color(const Bitmap& bmp){
  Optional<Color> c(resize_dwim_color(bmp,
    inflated(rect_from_size(bmp.GetSize()), 1)));
  assert(c.IsSet()); // resize_dwim_color does not fail if rectangle is valid
  return c.Get();
}

static bool get_horizontal_scanline_color(const Bitmap& bmp, int y, Color& result){
  result = get_color_raw(bmp, 0, y);
  for (int x = 1; x != bmp.m_w; x++){
    if (get_color_raw(bmp, x, y) != result){
      return false;
    }
  }
  return true;
}

static bool get_vertical_scanline_color(const Bitmap& bmp, int x, Color& result){
  result = get_color_raw(bmp, x, 0);
  for (int y = 1; y != bmp.m_h; y++){
    if (get_color_raw(bmp, x, y) != result){
      return false;
    }
  }
  return true;
}

bool get_bottom_edge_color(const Bitmap& bmp, Color& result){
  return get_horizontal_scanline_color(bmp, bmp.m_h - 1, result);
}

bool get_left_edge_color(const Bitmap& bmp, Color& result){
  return get_vertical_scanline_color(bmp, 0, result);
}

bool get_right_edge_color(const Bitmap& bmp, Color& result){
  return get_vertical_scanline_color(bmp, bmp.m_w - 1, result);
}

bool get_top_edge_color(const Bitmap& bmp, Color& result){
  return get_horizontal_scanline_color(bmp, 0, result);
}

AtMost2<IntRect> get_auto_crop_rectangles(const Bitmap& bmp){
  Color color1;
  if (get_top_edge_color(bmp, color1)){
    IntRect r1 = get_auto_crop_rect(bmp, color1);
    if (empty(r1)){
      // Blank image, the empty rectangle would cause everything to be
      // cropped away, return nothing instead
      return {};
    }
    Color color2;
    if (get_bottom_edge_color(bmp, color2) && color2 != color1){
      IntRect r2 = get_auto_crop_rect(bmp, color2);
      assert(!empty(r2));
      return {r1, r2};
    }
    else{
      return {r1};
    }
  }
  else if (get_left_edge_color(bmp, color1)){
    IntRect r1(get_auto_crop_rect(bmp, color1));
    assert(!empty(r1));

    Color color2;
    if (get_right_edge_color(bmp, color2) && color2 != color1){
      IntRect r2 = get_auto_crop_rect(bmp, color2);
      assert(!empty(r2));
      return {r1, r2};
    }
    return {r1};
  }
  else if (get_bottom_edge_color(bmp, color1)){
    IntRect r(get_auto_crop_rect(bmp, color1));
    assert(!empty(r));
    return {r};
  }
  else if (get_right_edge_color(bmp, color1)){
    IntRect r(get_auto_crop_rect(bmp, color1));
    assert(!empty(r));
    return {r};
  }
  return {};
}

AtMost2<IntRect> get_auto_crop_rectangles(const Optional<Bitmap>& bmp){
  return bmp.Visit(
    [](const Bitmap& bmp){
      return get_auto_crop_rectangles(bmp);
    },
    [](){
      return AtMost2<IntRect>();
    });
}

Optional<IntRect> find_color_extents(const Bitmap& bmp, const Color& c){
  int minX = bmp.m_w;
  int minY = bmp.m_h;
  int maxX = 0;
  int maxY = 0;
  for (int y = 0; y != bmp.m_h; y++){
    for (int x = 0; x != bmp.m_w; x++){
      if (get_color_raw(bmp, x, y) == c){

        minY = std::min(minY, y);
        maxY = std::max(maxY, y);

        minX = std::min(minX, x);
        maxX = std::max(maxX, x);

      }
    }
  }
  if (minX <= maxX && minY <= maxY){
    return {IntRect(IntPoint(minX, minY), IntSize(maxX - minX + 1,
      maxY - minY + 1))};
  }

  return {};
}

} // namespace
