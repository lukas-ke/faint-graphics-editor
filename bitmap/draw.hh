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

#ifndef FAINT_DRAW_HH
#define FAINT_DRAW_HH
#include "bitmap/paint.hh"
#include "geo/offsat.hh"
#include "util/distinct.hh"
#include "util/setting-id.hh" // For LineCap
#include "util/template-fwd.hh"

namespace faint{

enum class Axis;
enum class ScaleQuality;
class AlphaMapRef;
class Angle;
class Bitmap;
class Brush;
class IntLineSegment;
class IntRect;
class IntSize;
class Interval;
class Scale;
template<typename T> class Offsat;

using NewColor = Order<Color>::New;
using OldColor = Order<Color>::Old;

class category_axiality;
using along = Distinct<Axis, category_axiality, 0>;
using across = Distinct<Axis, category_axiality, 1>;

class DstBmp;

class LineSettings;

class BorderSettings{
public:
  BorderSettings(const LineSettings&);

  BorderSettings(const Paint& paint, int lineWidth, LineStyle style) :
    paint(paint), lineWidth(lineWidth), lineStyle(style)
  {}

  BorderSettings(const Paint&& paint, int lineWidth, LineStyle style) :
    paint(std::move(paint)), lineWidth(lineWidth), lineStyle(style)
  {}

  BorderSettings(const Color& color, int lineWidth, LineStyle style) :
    paint(color), lineWidth(lineWidth), lineStyle(style)
  {}

  bool Dashed() const{
    return lineStyle != LineStyle::SOLID;
  }

  Paint paint;
  int lineWidth;
  LineStyle lineStyle;
};

class LineSettings{
public:
  LineSettings(const Paint& p, int lineWidth, LineStyle style, LineCap cap)
    : paint(p), lineWidth(lineWidth), lineStyle(style), cap(cap)
  {}

  LineSettings(const Color& c, int lineWidth, LineStyle style, LineCap cap)
    : paint(c), lineWidth(lineWidth), lineStyle(style), cap(cap)
  {}

  LineSettings(const BorderSettings& b, LineCap cap)
    : paint(b.paint), lineWidth(b.lineWidth), lineStyle(b.lineStyle), cap(cap)
  {}

  LineSettings(const BorderSettings&& b, LineCap cap)
    : paint(std::move(b.paint)), lineWidth(b.lineWidth), lineStyle(b.lineStyle),
      cap(cap)
  {}
  Paint paint;
  int lineWidth;
  LineStyle lineStyle;
  LineCap cap;
};

LineSettings solid_1px(const Color&);
LineSettings with_long_dash(LineSettings);

Bitmap alpha_blended(const Bitmap&, const Color&);
Bitmap alpha_blended(const Bitmap&, const ColRGB&);
void blend_alpha(Bitmap&, const ColRGB&);
void blend(const Offsat<Bitmap>&, DstBmp);
void blend_masked(const Offsat<Bitmap>& src, DstBmp, const Color& maskColor);
void blend(const Offsat<AlphaMapRef>&, DstBmp, const Paint&);
void blit(const Offsat<Bitmap>&, DstBmp);
void blit_masked(const Offsat<Bitmap>& src, DstBmp, const Color&);
void boundary_fill(Bitmap&, const IntPoint&, const Paint& fill,
  const Color& boundaryColor);
Brush circle_brush(int w);
void clear(Bitmap&, const Color&);
void draw_ellipse(Bitmap&, const IntRect&, const BorderSettings&);
void draw_line(Bitmap&, const IntLineSegment&, const LineSettings&);
void draw_hline(Bitmap&, int y, const std::pair<int, int>& x,
  const LineSettings&);
void draw_vline(Bitmap&, int x, const std::pair<int, int>& y,
  const LineSettings&);
void draw_polygon(Bitmap&, const std::vector<IntPoint>&, const BorderSettings&);
void draw_polyline(Bitmap&, const std::vector<IntPoint>&, const LineSettings&);
void draw_rect(Bitmap&, const IntRect&, const BorderSettings&);
void erase_but(Bitmap&, const Color& keep, const Paint& eraser);
void fill_ellipse(Bitmap&, const IntRect&, const Paint&);
void fill_ellipse_color(Bitmap&, const IntRect&, const Color&);
void fill_polygon(Bitmap&, const std::vector<IntPoint>&, const Paint&);
void fill_rect(Bitmap&, const IntRect&, const Paint&);
void fill_rect_color(Bitmap&, const IntRect&, const Color&);
void fill_rect_rgb(Bitmap&, const IntRect&, const ColRGB&);
void fill_triangle(Bitmap&, const IntPoint&, const IntPoint&, const IntPoint&,
  const Paint&);
void fill_triangle_color(Bitmap&, const IntPoint&, const IntPoint&,
  const IntPoint&, const Color&);
Bitmap flip(const Bitmap&, const along&);
Bitmap flip(const Bitmap&, const across&);
void flood_fill_color(Bitmap&, const IntPoint&, const Color& fillColor);
void flood_fill(Bitmap&, const IntPoint&, const Paint&);
std::vector<Color> get_palette(const Bitmap&);
bool intersects(const Offsat<AlphaMapRef>&, const Bitmap&);
bool intersects(const Offsat<Bitmap>&, const DstBmp&);
bool overextends_down(const IntRect&, const Bitmap&);
bool overextends_left(const IntRect&, const Bitmap&);
bool overextends_right(const IntRect&, const Bitmap&);
bool overextends_up(const IntRect&, const Bitmap&);
void put_pixel(Bitmap&, const IntPoint&, const Color&);
void put_pixel_raw(Bitmap&, int x, int y, const Color&);
void replace_color(Bitmap&, const OldColor&, const Paint&);
void replace_color_color(Bitmap&, const OldColor&, const NewColor&);
void replace_color_pattern(Bitmap&, const OldColor&, const Bitmap& pattern);
void rect(Bitmap&, const IntRect&, const Optional<BorderSettings>&,
  const Optional<Paint>& bg);
Bitmap rotate_nearest(const Bitmap&, const Angle&, const Color& bg);
Bitmap rotate_bilinear(const Bitmap&, const Angle&, const Paint& bg);
Bitmap rotate_bilinear(const Bitmap&, const Angle&);
Bitmap rotate_90cw(const Bitmap&);
IntSize rotate_scale_bilinear_size(const IntSize& oldSize,
  const Angle&, const Scale&);
Bitmap rotate_scale_bilinear(const Bitmap&, const Angle&, const Scale&,
  const Paint& bg);
Bitmap scale(const Bitmap&, const Scale&, ScaleQuality);
Bitmap scale_bilinear(const Bitmap&, const Scale&);
Bitmap scale_nearest(const Bitmap&, int scale);
Bitmap scale_nearest(const Bitmap&, const Scale&);
Bitmap scaled_subbitmap(const Bitmap&, const Scale&, const IntRect&);
void set_alpha(Bitmap&, uchar);
Bitmap subbitmap(const Bitmap&, const IntRect&);
void vertical_scanline(Bitmap&, int x, const Color&);
void horizontal_scanline(Bitmap&, int y, const Color&);

} // namespace

#endif
