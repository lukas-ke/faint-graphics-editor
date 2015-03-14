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

#include "bitmap/aa-line.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/draw.hh"
#include "geo/line.hh"
#include "geo/int-point.hh"
#include "geo/primitive.hh"

namespace faint{

coord fpart(coord v){
  return v - floated(truncated(v));
}

coord rfpart(coord v){
  return 1 - fpart(v);
}

inline uchar blend_value(uchar oldValue, uchar newValue, double intensity){
  return static_cast<uchar>(oldValue * (1.0 - intensity) + newValue * intensity);
}

static void blend_pixel(Bitmap& bmp,
  int x,
  int y,
  const ColRGB& c,
  double intensity)
{
  if (x < 0 || y < 0 || x >= bmp.m_w || y >= bmp.m_h){
      return;
  }
  Color old(get_color_raw(bmp, x,y));
  put_pixel_raw(bmp, x, y, Color(
    blend_value(old.r, c.r, intensity),
    blend_value(old.g, c.g, intensity),
    blend_value(old.b, c.b, intensity),
    255));
}

void draw_line_aa_Wu(Bitmap& bmp, const IntLineSegment& l, const ColRGB& c){
  if (l.p0 == l.p1){
    put_pixel(bmp, l.p0, Color(c, 255));
    return;
  }

  // Reorder coordinates if necessary for simplifying the algorithm
  IntPoint p0(l.p0);
  IntPoint p1(l.p1);
  bool steep = std::abs(p1.y - p0.y)  > std::abs(p1.x - p0.x);
  if (steep){
    std::swap(p0.x, p0.y);
    std::swap(p1.x, p1.y);
  }
  if (p0.x > p1.x){
    std::swap(p0, p1);
  }

  int dx = p1.x - p0.x;
  int dy = p1.y - p0.y;
  const coord gradient = dy / floated(dx);
  coord xend = p0.x;
  coord yend = p0.y + gradient * (xend - p0.x);
  coord xgap = rfpart(p0.x + 0.5);
  int x1 = static_cast<int>(xend);
  int y1 = static_cast<int>(yend);

  // First end point
  if (steep){
    blend_pixel(bmp, y1, x1, c, rfpart(yend) * xgap);
    blend_pixel(bmp, y1 + 1, x1, c, fpart(yend) * xgap);
  }
  else{
    blend_pixel(bmp, x1, y1, c, rfpart(yend) * xgap);
    blend_pixel(bmp, x1, y1 + 1, c, fpart(yend) * xgap);
  }
  // First y-intersection for the main-loop
  coord y_inter = yend + gradient;

  // Second end point
  xend = p1.x;
  yend = p1.y + gradient * (xend - p1.x);
  xgap = fpart(p1.x + 0.5);
  int x2 = static_cast<int>(xend);
  int y2 = static_cast<int>(yend);
  if (steep){
    blend_pixel(bmp, y2, x2, c, rfpart(yend) * xgap);
    blend_pixel(bmp, y2 + 1, x2, c, fpart(yend) * xgap);
  }
  else{
    blend_pixel(bmp, x2, y2, c, rfpart(yend) * xgap);
    blend_pixel(bmp, x2, y2+1, c, fpart(yend) * xgap);
  }

  for (int x = x1 + 1; x != x2; x++){
    if (steep){
      blend_pixel(bmp, int(y_inter), x, c, rfpart(y_inter));
      blend_pixel(bmp, int(y_inter) + 1, x, c, fpart(y_inter));
    }
    else{
      blend_pixel(bmp, x, int(y_inter), c, rfpart(y_inter));
      blend_pixel(bmp, x, int(y_inter) + 1, c, fpart(y_inter));
    }
    y_inter += gradient;
  }
}

} // namespace
