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

#ifndef FAINT_CAIRO_CONTEXT_HH
#define FAINT_CAIRO_CONTEXT_HH
#include <array>
#include <string>
#include "bitmap/bitmap-fwd.hh"
#include "geo/primitive.hh"
#include "text/text-measures.hh"

namespace faint{

enum class LineCap;
enum class LineJoin;
class Angle;
class AngleSpan;
class Bitmap;
class Color;
class Gradient;
class IntSize;
class Paint;
class PathPt;
class Pattern;
class Settings;
class Tri;
class utf8_string;

Bitmap cairo_skew(const Bitmap&, coord skew, coord skew_pixels);
Bitmap cairo_gradient_bitmap(const Gradient&, const IntSize&);
std::string get_cairo_version();
std::string get_pango_version();

class CairoContextImpl;
class CairoContext{
public:
  CairoContext(Bitmap&);
  ~CairoContext();

  bool IsTargetting(const Bitmap& bmp) const;
  bool IsOk() const;
  std::string ErrorString() const;
  void set_clip_polygon(const std::array<Point,4>&);
  void close_path();

  void arc(const Point& center, coord r, const AngleSpan&);
  void arc_negative(const Point& center, coord r, const AngleSpan&);

  // Cubic bezier spline to end using c1 and c2 as control points
  void curve_to(const Point& c1, const Point& c2, const Point& end);
  void fill();
  void fill_preserve();
  void line_to(const Point&);
  void move_to(const Point&);
  void pango_text(const Tri&, const utf8_string&, const Settings&);
  std::vector<PathPt> get_text_path(const Tri&, const utf8_string&,
    const Settings&);
  Size pango_text_size(const utf8_string&, const Settings&) const;
  TextMeasures pango_text_extents(const utf8_string&, const Settings&) const;
  FontMetrics pango_font_metrics(const Settings&) const;
  std::vector<int> cumulative_text_width(const utf8_string&,
    const Settings&) const;
  void restore();
  void rotate(const Angle&);
  void save();
  void scale(coord, coord);
  void set_dash(const coord* dashes, int num, double offset);
  void set_line_cap(LineCap);
  void set_line_join(LineJoin);
  void set_source_tri(const Tri&);
  void set_source(const Paint&);
  void set_line_width(coord);
  void stroke();
  void translate(const Point&);

  void new_sub_path();

  CairoContext(const CairoContext&) = delete;
  CairoContext& operator=(const CairoContext&) = delete;
private:
  void set_source_surface(const Pattern&);
  void set_source_rgba(const Color&);
  void set_source_gradient(const Gradient&);
  CairoContextImpl* m_impl;
};

class CairoSave{
  // RAII-class for restoring Cairo-transforms at end of scope
public:
  explicit CairoSave(CairoContext&);
  ~CairoSave();
  CairoSave& operator=(const CairoSave&) = delete;
private:
  CairoContext& m_context;
};


} // namespace

#endif
