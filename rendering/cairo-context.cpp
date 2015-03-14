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
#include "bitmap/bitmap.hh"
#include "bitmap/gradient.hh"
#include "bitmap/paint.hh"
#include "bitmap/pattern.hh"
#include "bitmap/rotate-util.hh"
#include "geo/arc.hh"
#include "geo/geo-func.hh"
#include "geo/pathpt.hh"
#include "geo/scale.hh"
#include "geo/size.hh"
#include "geo/tri.hh"
#include "rendering/cairo-context.hh"
#include "rendering/cairo-ptr.hh"
#include "text/utf8-string.hh"
#include "util/index-iter.hh"
#include "util/iter.hh"
#include "util/settings.hh"
#include "util/setting-id.hh"

namespace faint{

static font_description_ptr_t get_font_description(const Settings& s){
  auto fd(manage(pango_font_description_new()));

  pango_font_description_set_family(fd.get(),
    s.Get(ts_FontFace).c_str());

  pango_font_description_set_weight(fd.get(),
    s.GetDefault(ts_FontBold, false) ?
    PANGO_WEIGHT_BOLD : PANGO_WEIGHT_NORMAL);

  pango_font_description_set_style(fd.get(),
    s.GetDefault(ts_FontItalic, false) ?
    PANGO_STYLE_ITALIC : PANGO_STYLE_NORMAL);

  pango_font_description_set_absolute_size(fd.get(),
    s.Get(ts_FontSize) * PANGO_SCALE);

  return fd;
}

// Fixme: Should use this instead of getting extents of
// a character (like 'M') for calculations.
// Fixme: HOWEVER: Make this more efficient, see descent f.x
static int get_font_ascent(const Settings& s){
  auto fd(get_font_description(s));
  auto fontMap(manage(pango_cairo_font_map_get_default()));
  auto ctx(manage(pango_font_map_create_context(fontMap.get())));
  auto font(manage(pango_font_map_load_font(fontMap.get(), ctx.get(),
    fd.get())));
  auto metrics(manage(pango_font_get_metrics(font.get(), NULL)));

  int ascent = pango_font_metrics_get_ascent(metrics.get()) / PANGO_SCALE;
  return ascent;
}

// Fixme: Combine with get_font_ascent
static int get_font_descent(const Settings& s){
  auto fd(get_font_description(s));
  auto fontMap(manage(pango_cairo_font_map_get_default()));
  auto ctx(manage(pango_font_map_create_context(fontMap.get())));
  auto font(manage(pango_font_map_load_font(fontMap.get(), ctx.get(),
    fd.get())));
  auto metrics(manage(pango_font_get_metrics(font.get(), NULL)));

  int ascent = pango_font_metrics_get_descent(metrics.get()) / PANGO_SCALE;
  return ascent;
}

static Rect to_faint(const PangoRectangle& r){
  return Rect(Point(r.x, r.y), Size(r.width, r.height));
}

static cairo_matrix_t faint_cairo_gradient_matrix(const LinearGradient& g, const Size& sz){
  cairo_matrix_t m;
  cairo_matrix_init_identity(&m);
  cairo_matrix_translate(&m, 0.5, 0.5);
  cairo_matrix_rotate(&m, -g.GetAngle().Rad());
  cairo_matrix_translate(&m, -0.5, -0.5);
  cairo_matrix_scale(&m, 1.0 / sz.w, 1.0 / sz.h); // Fixme: What of 0 size
  return m;
}

static cairo_matrix_t faint_cairo_gradient_matrix(const RadialGradient& g, const Size& sz){
  cairo_matrix_t m;
  cairo_matrix_init_identity(&m);
  cairo_matrix_scale(&m, 1.0 / sz.w, 1.0 / sz.h); // Fixme: What of 0 size
  Point center(g.GetCenter());
  cairo_matrix_translate(&m, -center.x, center.y);
  return m;
}

static surface_ptr_t get_cairo_surface(Bitmap& bmp){
  return manage(cairo_image_surface_create_for_data(bmp.GetRaw(),
      CAIRO_FORMAT_ARGB32,
      bmp.m_w,
      bmp.m_h,
      bmp.m_row_stride));
}

static pattern_ptr_t faint_cairo_linear_gradient(const LinearGradient& g){
  auto cg(manage(cairo_pattern_create_linear(0.0, 0.0, 1.0, 0.0)));

  for (auto i : up_to(g.GetNumStops())){
    ColorStop stop(g.GetStop(i));
    Color c(stop.GetColor());
    double offset(stop.GetOffset());
    cairo_pattern_add_color_stop_rgba(cg, offset,
      c.r / 255.0, c.g / 255.0, c.b / 255.0, c.a / 255.0);
  }
  return cg;
}

static pattern_ptr_t faint_cairo_radial_gradient(const RadialGradient& g){
  auto cg(manage(cairo_pattern_create_radial(
      0.5, 0.5, 0.0, // Inner circle
      0.5, 0.5, 0.5))); // Outer

  for (auto i : up_to(g.GetNumStops())){
    ColorStop stop(g.GetStop(i));
    Color c(stop.GetColor());
    double offset(stop.GetOffset());
    cairo_pattern_add_color_stop_rgba(cg,
      offset, c.r / 255.0, c.g / 255.0, c.b / 255.0, c.a / 255.0);
  }
  return cg;
}

static Bitmap cairo_linear_gradient_bitmap(const LinearGradient& gradient,
  const IntSize& sz)
{
  auto p_gradient(faint_cairo_linear_gradient(gradient));
  Bitmap bmpDst(sz, color_transparent_white);
  surface_ptr_t s_dst(get_cairo_surface(bmpDst));
  auto cr(cairo_create(s_dst));

  cairo_matrix_t m(faint_cairo_gradient_matrix(gradient, floated(sz)));
  cairo_pattern_set_matrix(p_gradient, &m);
  cairo_set_source(cr, p_gradient);

  // Fixme: Consider cairo_mask to draw gradient
  cairo_move_to(cr, 0, 0);
  cairo_line_to(cr, sz.w, 0);
  cairo_line_to(cr, sz.w, sz.h);
  cairo_line_to(cr, 0, sz.h);
  cairo_close_path(cr);
  cairo_fill(cr);
  return bmpDst;
}

static Bitmap cairo_radial_gradient_bitmap(const RadialGradient& gradient,
  const IntSize& sz)
{
  auto p_gradient(faint_cairo_radial_gradient(gradient));
  Bitmap bmpDst(sz, color_transparent_white);
  auto s_dst(get_cairo_surface(bmpDst));
  auto cr(cairo_create(s_dst));
  cairo_matrix_t m(faint_cairo_gradient_matrix(gradient, floated(sz)));
  cairo_pattern_set_matrix(p_gradient, &m);
  cairo_set_source(cr, p_gradient);

  cairo_move_to(cr, 0, 0);
  cairo_line_to(cr, sz.w, 0);
  cairo_line_to(cr, sz.w, sz.h);
  cairo_line_to(cr, 0, sz.h);
  cairo_close_path(cr);
  cairo_fill(cr);
  return bmpDst;
}

Bitmap cairo_gradient_bitmap(const Gradient& gradient,
  const IntSize& sz)
{
  if (gradient.IsLinear()){
    return cairo_linear_gradient_bitmap(gradient.GetLinear(), sz);
  }
  else{
    return cairo_radial_gradient_bitmap(gradient.GetRadial(), sz);
  }
}

// Returns a new bitmap of Size sz, filled with bg which in_bmpSrc is
// transformed onto using the passed in transformation function.
template<typename Func>
Bitmap transform_copy(const Bitmap& in_bmpSrc, const IntSize& sz,
  const Paint& bg, const Point& offset, const Func& transform)
{
  // Needs non-const bitmap for source surface. Will not modify.
  Bitmap& bmpSrc(const_cast<Bitmap&>(in_bmpSrc));
  Bitmap bmpDst(sz, bg);
  auto dstSurface(get_cairo_surface(bmpDst));
  auto srcSurface(get_cairo_surface(bmpSrc));

  auto cr(cairo_create(dstSurface));

  transform(cr.get());

  // Clip so that only pixels from the source affects the target. The
  // background of new pixels should be determined by the bg
  // parameter.
  //
  // Fixme: Uses a slightly smaller clip than the actual destination
  // region to avoid blending towards alpha, but this loses pixels.
  // See \ref(rotation-blending)
  cairo_rectangle(cr, offset.x + 1, offset.y + 1,
    bmpSrc.m_w - 2, bmpSrc.m_h - 2);

  cairo_clip(cr);

  // Transfer to destination
  cairo_set_operator(cr,
    // Don't blend alpha, set it
    CAIRO_OPERATOR_SOURCE);
  cairo_set_source_surface(cr, srcSurface, offset.x, offset.y);
  cairo_paint(cr);

  return bmpDst;
}

Bitmap cairo_skew(const Bitmap& src, coord skew_angle, coord skew_pixels){
  IntSize size(int(src.m_w + std::fabs(skew_pixels)), src.m_h);
  return transform_copy(src, size, Paint(color_transparent_black),
    Point(0,0),
    [&](cairo_t* cr){
      cairo_matrix_t m_skew;
      cairo_matrix_init(&m_skew, 1,0,skew_angle,1,0,0);
      if (skew_pixels > 0){
        cairo_matrix_t m_translate;
        cairo_matrix_init(&m_translate, 1, 0, 0, 1, skew_pixels, 0);
        cairo_matrix_multiply(&m_skew, &m_translate, &m_skew);
      }
      cairo_set_matrix(cr, &m_skew);
    });
}

Bitmap rotate_bilinear(const Bitmap& src, const Angle& angle, const Paint& bg){
  RotationAdjustment adj = get_rotation_adjustment(angle, src.GetSize());
  Point offset(-src.m_w / 2.0, -src.m_h / 2.0);
  return transform_copy(src, adj.size, bg, offset,
    [&](cairo_t* cr){
      cairo_translate(cr, adj.size.w / 2.0, adj.size.h / 2.0);
      cairo_rotate(cr, angle.Rad());
    });
}

Bitmap rotate_bilinear(const Bitmap& bmp, const Angle& angle){
  return rotate_bilinear(bmp, angle, Paint(color_transparent_white));
}

IntSize rotate_scale_bilinear_size(const IntSize& size,
  const Angle& angle,
  const Scale& scale)
{
  RotationAdjustment adj = get_rotation_adjustment(angle, size);
  return rounded(floated(adj.size) * scale);
}

Bitmap rotate_scale_bilinear(const Bitmap& src, const Angle& angle,
  const Scale& scale, const Paint& bg)
{
  IntSize newSize(rotate_scale_bilinear_size(src.GetSize(), angle, scale));
  Point offset(-src.m_w / 2.0, -src.m_h / 2.0);
  return transform_copy(src, newSize, bg,
    offset,
    [&](cairo_t* cr){
      cairo_translate(cr, newSize.w / 2.0, newSize.h / 2.0);
      cairo_rotate(cr, angle.Rad());
      cairo_scale(cr, scale.x, scale.y);
    });
}

static cairo_matrix_t cairo_linear_matrix_from_tri(const Tri& t,
  const Angle& angle,
  bool objectAligned)
{
  double sx = t.Width();
  double sy = t.Height();
  double x0 = t.P0().x;
  double y0 = t.P0().y;

  if (!objectAligned){
    Rect r(bounding_rect(t));
    sx = r.w;
    sy = r.h;
    x0 = r.x;
    y0 = r.y;
  }
  cairo_matrix_t m;
  cairo_matrix_init_identity(&m);
  cairo_matrix_translate(&m, 0.5, 0.5);
  cairo_matrix_rotate(&m, -angle.Rad());
  cairo_matrix_translate(&m, -0.5, -0.5);
  cairo_matrix_scale(&m, 1.0 / sx, 1.0 / sy);
  if (objectAligned){
    cairo_matrix_rotate(&m, -t.GetAngle().Rad());
  }
  cairo_matrix_translate(&m, -x0, -y0);
  return m;
}

static cairo_matrix_t cairo_radial_matrix_from_tri(const Tri& t,
  const RadialGradient& g)
{
  if (g.GetObjectAligned()){
    double sx = t.Width();
    double sy = t.Height();
    Point center(g.GetCenter());
    double x0 = t.P0().x;
    double y0 = t.P0().y;
    cairo_matrix_t m;
    cairo_matrix_init_identity(&m);
    cairo_matrix_translate(&m, center.x, center.y);
    cairo_matrix_scale(&m, 1.0 / sx, 1.0 / sy);
    cairo_matrix_translate(&m, -center.x, -center.y);
    cairo_matrix_translate(&m, -x0, -y0);
    return m;
  }
  else {
    Radii r(g.GetRadii());
    Point c(g.GetCenter());
    cairo_matrix_t m;
    cairo_matrix_init_identity(&m);
    cairo_matrix_translate(&m, 0.5, 0.5);
    cairo_matrix_scale(&m, 1.0 / (2 * r.x), 1.0 / (2 * r.y));
    cairo_matrix_translate(&m, -0.5, -0.5);
    cairo_matrix_translate(&m, -c.x, -c.y);
    return m;
  }
}

static cairo_matrix_t cairo_pattern_matrix_from_tri(const Tri& t,
  const Point& offset)
{
  Point p0(t.P0());
  cairo_matrix_t m;
  cairo_matrix_init_identity(&m);
  cairo_matrix_translate(&m, -p0.x + offset.x, -p0.y + offset.y);
  return m;
}

static cairo_matrix_t translate_matrix(const Point& delta){
  cairo_matrix_t m;
  cairo_matrix_init_identity(&m);
  cairo_matrix_translate(&m, delta.x, delta.y);
  return m;
}

std::string get_cairo_version(){
  return CAIRO_VERSION_STRING; // Note: Compile time.
}

std::string get_pango_version(){
  return PANGO_VERSION_STRING; // Note: Compile time.
}

class CairoContextImpl{
public:
  CairoContextImpl(Bitmap& bmp)
    : patternTri(Point(0,0), Point(100,0), 100.0)
  {
    this->surface = get_cairo_surface(bmp);
    this->cr = cairo_create(surface);
    this->srcBmp = nullptr;
    this->srcPattern = nullptr;
    this->gradientPattern = nullptr;
    this->linearGradient = nullptr;
    this->radialGradient = nullptr;
    this->pattern = nullptr;
    this->rawAddress = bmp.GetRaw();
  }
  ~CairoContextImpl(){
    if (this->srcSurface != nullptr){
      delete this->srcBmp;
    }
    delete this->linearGradient;
    delete this->radialGradient;
    delete this->pattern;
  }
  cairo_ptr_t cr;
  surface_ptr_t surface;

  Tri patternTri;

  Bitmap* srcBmp;
  surface_ptr_t srcSurface;
  pattern_ptr_t srcPattern;
  Pattern* pattern;

  pattern_ptr_t gradientPattern;
  LinearGradient* linearGradient;
  RadialGradient* radialGradient;
  const uchar* rawAddress;
};

CairoContext::CairoContext(Bitmap& bmp)
  : m_impl(new CairoContextImpl(bmp))
{}

CairoContext::~CairoContext(){
  delete m_impl;
}

bool CairoContext::IsTargetting(const Bitmap& bmp) const{
  return m_impl->rawAddress == bmp.GetRaw();
}

bool CairoContext::IsOk() const{
  return cairo_status(m_impl->cr.get()) == CAIRO_STATUS_SUCCESS;
}

std::string CairoContext::ErrorString() const{
  return cairo_status_to_string(cairo_status(m_impl->cr.get()));
}

void CairoContext::close_path(){
  cairo_close_path(m_impl->cr);
}

void CairoContext::arc(const Point& c, coord r, const AngleSpan& angles){
  cairo_arc(m_impl->cr, c.x, c.y, r, angles.start.Rad(), angles.stop.Rad());
}

void CairoContext::arc_negative(const Point& c, coord r, const AngleSpan& angles){
  cairo_arc_negative(m_impl->cr, c.x, c.y, r,
    angles.start.Rad(), angles.stop.Rad());
}

void CairoContext::curve_to(const Point& p1, const Point& p2, const Point& p3){
  cairo_curve_to(m_impl->cr, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
}

void CairoContext::fill(){
  cairo_fill(m_impl->cr);
}

void CairoContext::fill_preserve(){
  cairo_fill_preserve(m_impl->cr);
}

void CairoContext::line_to(const Point& p){
  cairo_line_to(m_impl->cr, p.x, p.y);
}

void CairoContext::move_to(const Point& p){
  cairo_move_to(m_impl->cr, p.x, p.y);
}

static layout_ptr_t get_text_layout(cairo_ptr_t& cr,
  font_description_ptr_t& fd,
  const utf8_string& s)
{
  auto layout(manage(pango_cairo_create_layout(cr.get())));
  pango_layout_set_font_description(layout, fd);
  pango_layout_set_text(layout.get(), s.c_str(), -1);
  return layout;
}

void CairoContext::pango_text(const Tri& t,
  const utf8_string& text,
  const Settings& s)
{
  set_source(s.Get(ts_Fg)); // Fixme
  save();

  auto fontOptions(manage(cairo_font_options_create()));
  cairo_font_options_set_antialias(fontOptions.get(), CAIRO_ANTIALIAS_SUBPIXEL);
  cairo_font_options_set_hint_metrics(fontOptions.get(), CAIRO_HINT_METRICS_ON);
  cairo_font_options_set_hint_style(fontOptions.get(), CAIRO_HINT_STYLE_FULL);
  cairo_set_font_options(m_impl->cr.get(), fontOptions.get());

  auto fd(get_font_description(s));
  auto layout(get_text_layout(m_impl->cr, fd, text));

  translate(t.P0());
  rotate(t.GetAngle());
  if (t.Height() < 0){
    scale(1, -1);
  }

  // Offset to anchor at the top of the text instead of the baseline.
  translate(Point(0, get_font_ascent(s)));

  PangoLayoutLine* line = pango_layout_get_line_readonly(layout.get(), 0);

  TextRenderStyle renderStyle = s.Get(ts_TextRenderStyle);
  bool renderAsPath = renderStyle == TextRenderStyle::CAIRO_PATH ||
    (renderStyle == TextRenderStyle::PANGO_LAYOUT_UNLESS_ROTATED &&
      !rather_zero(t.GetAngle()));

  if (renderAsPath){
    pango_cairo_layout_line_path(m_impl->cr.get(), line);
    fill();
  }
  else{
    // Rendering with Pango gives somewhat nicer output on Windows
    // (e.g. ClearType sub-pixel anti aliasing), but the letters
    // "wobble" if the text is rotated. Also cleartype isn't nice when
    // flattening/saving as raster image.
    pango_cairo_show_layout_line(m_impl->cr.get(), line);
  }
  restore();
}

std::vector<PathPt> CairoContext::get_text_path(const Tri& t,
  const utf8_string& text,
  const Settings& s)
{
  // Fixme: Lots of the following duplicates text drawing!

  set_source(s.Get(ts_Fg)); // Fixme
  save();
  const Point p0(t.P0());
  auto fd(get_font_description(s));

  auto layout(get_text_layout(m_impl->cr, fd, text));

  translate(p0);
  rotate(t.GetAngle());
  if (t.Height() < 0){
    scale(1, -1);
  }

  // Offset to anchor at the top
  Point offset(0, get_font_ascent(s));
  translate(offset);

  PangoLayoutLine* line = pango_layout_get_line(layout.get(), 0);
  pango_cairo_layout_line_path(m_impl->cr.get(), line);

  std::vector<PathPt> path;
  auto cairoPath(manage(cairo_copy_path(m_impl->cr.get())));

  Point delta = p0 + offset;
  for (int i = 0; i < cairoPath->num_data; i += cairoPath->data[i].header.length){
    cairo_path_data_t* data = &cairoPath->data[i];
    switch (data->header.type) {
    case CAIRO_PATH_MOVE_TO:
      path.push_back(PathPt::MoveTo(Point(data[1].point.x, data[1].point.y) +
        delta));
      break;
    case CAIRO_PATH_LINE_TO:
      path.push_back(PathPt::LineTo(Point(data[1].point.x, data[1].point.y) + delta));
      break;
    case CAIRO_PATH_CURVE_TO:
      path.push_back(PathPt::CubicBezierTo(
        Point(data[3].point.x, data[3].point.y) + delta,
        Point(data[1].point.x, data[1].point.y) + delta,
        Point(data[2].point.x, data[2].point.y) + delta));
      break;
    case CAIRO_PATH_CLOSE_PATH:
      path.push_back(PathPt::PathCloser());
      break;
    }
  }

  restore();
  return path;
}

void CairoContext::new_sub_path(){
  cairo_new_sub_path(m_impl->cr);
}

Size CairoContext::pango_text_size(const utf8_string& text,
  const Settings& s) const
{
  auto fd(get_font_description(s));
  auto layout(manage(pango_cairo_create_layout(m_impl->cr.get())));
  pango_layout_set_font_description(layout, fd);
  utf8_string useText;
  if (text.size() > 1){
    useText = text.substr(0, text.size() - 2);
  }
  else{
    useText = text;
  }
  pango_layout_set_text(layout.get(), text.c_str(), -1);

  int w, h;
  pango_layout_get_pixel_size(layout.get(), &w, &h);
  return Size(w, h);
}

FontMetrics CairoContext::pango_font_metrics(const Settings& s) const{
  auto fd(get_font_description(s));
  FontMetrics m;
  m.ascent = get_font_ascent(s);
  m.descent = get_font_descent(s);
  return m;
}

TextMeasures CairoContext::pango_text_extents(const utf8_string& text,
  const Settings& s) const
{
  auto fd(get_font_description(s));
  auto layout(manage(pango_cairo_create_layout(m_impl->cr.get())));
  pango_layout_set_font_description(layout, fd);
  utf8_string useText;
  if (text.size() > 1){
    useText = text.substr(0, text.size() - 2);
  }
  else{
    useText = text;
  }
  pango_layout_set_text(layout.get(), text.c_str(), -1);
  PangoRectangle ink = {0,0,0,0};
  PangoRectangle logical = {0,0,0,0};
  pango_layout_get_pixel_extents(layout.get(),
    &ink,
    &logical);
  TextMeasures result;
  result.ink = to_faint(ink);
  result.logical = to_faint(logical);
  return result;
}

std::vector<int> CairoContext::cumulative_text_width(const utf8_string& text,
  const Settings& s) const
{
  auto fd(get_font_description(s));
  auto layout(manage(pango_cairo_create_layout(m_impl->cr.get())));
  pango_layout_set_font_description(layout, fd);

  std::vector<int> v;
  v.push_back(0);
  for (size_t i = 1; i <= text.size(); i++){
    utf8_string substr(text.substr(0, i));
    int w, h;
    pango_layout_set_text(layout.get(), substr.c_str(), -1);
    pango_layout_get_pixel_size(layout.get(), &w, &h);
    v.push_back(w);
  }
  return v;
}

void CairoContext::restore(){
  cairo_restore(m_impl->cr);
}

void CairoContext::rotate(const Angle& angle){
  cairo_rotate(m_impl->cr, angle.Rad());
}

void CairoContext::save(){
  cairo_save(m_impl->cr);
}

void CairoContext::scale(coord sx, coord sy){
  cairo_scale(m_impl->cr, sx, sy);
}

void CairoContext::set_clip_polygon(const std::array<Point,4>& pts){
  // Fixme: This doesn't work quite right, maybe
  // https://bugzilla.gnome.org/show_bug.cgi?id=697357
  move_to(pts.front());
  for(const Point& pt : but_first(pts)){
    line_to(pt);
  }
  close_path();
  cairo_clip(m_impl->cr);
}

void CairoContext::set_dash(const coord* dashes, int num, double offset){
  cairo_set_dash(m_impl->cr.get(), dashes, num, offset);
}

void CairoContext::set_line_cap(LineCap cap){
  if (cap == LineCap::ROUND){
    cairo_set_line_cap(m_impl->cr.get(), CAIRO_LINE_CAP_ROUND);
  }
  else if (cap == LineCap::BUTT){
    cairo_set_line_cap(m_impl->cr.get(), CAIRO_LINE_CAP_BUTT);
  }
  else {
    assert(false);
  }
}

void CairoContext::set_line_join(LineJoin join){
  if (join == LineJoin::ROUND){
    cairo_set_line_join(m_impl->cr.get(), CAIRO_LINE_JOIN_ROUND);
  }
  else if (join == LineJoin::BEVEL){
    cairo_set_line_join(m_impl->cr.get(), CAIRO_LINE_JOIN_BEVEL);
  }
  else if (join == LineJoin::MITER){
    cairo_set_line_join(m_impl->cr.get(), CAIRO_LINE_JOIN_MITER);
  }
  else {
    assert(false);
  }
}

void CairoContext::set_line_width(coord width){
  cairo_set_line_width(m_impl->cr.get(), width);
}

void CairoContext::set_source(const Paint& paint){
  visit(paint,
    [&](const Color& color){set_source_rgba(color);},
    [&](const Pattern& pattern){set_source_surface(pattern);},
    [&](const Gradient& gradient){set_source_gradient(gradient);});
}

void CairoContext::set_source_rgba(const Color& c){
  cairo_set_source_rgba(m_impl->cr,
    c.r / 255.0,
    c.g / 255.0,
    c.b / 255.0,
    c.a / 255.0);

  if (m_impl->srcSurface != nullptr){
    m_impl->srcSurface.reset(nullptr);
    m_impl->srcPattern.reset(nullptr);
    delete m_impl->srcBmp;
    m_impl->srcBmp = nullptr;
  }

  if (m_impl->gradientPattern != nullptr){
    m_impl->gradientPattern.reset(nullptr);
    delete m_impl->linearGradient;
    m_impl->linearGradient = nullptr;
    delete m_impl->radialGradient;
    m_impl->radialGradient = nullptr;
  }
}

void CairoContext::set_source_surface(const Pattern& pattern){
  if (m_impl->srcSurface != nullptr){
    m_impl->srcSurface.reset(nullptr);
    m_impl->srcPattern.reset(nullptr);
    delete m_impl->pattern;
    delete m_impl->srcBmp;
    m_impl->pattern = nullptr;
    m_impl->srcBmp = nullptr;
  }
  if (m_impl->gradientPattern != nullptr){
    m_impl->gradientPattern.reset(nullptr);
    delete m_impl->linearGradient;
    m_impl->linearGradient = nullptr;
    delete m_impl->radialGradient;
    m_impl->radialGradient = nullptr;
  }

  // Fixme: Probably no need to copy bitmap AND pattern.
  // Just clone the pattern, and use GetBitmap(), storing the pointer
  m_impl->srcBmp = new Bitmap(pattern.GetBitmap()); // Fixme: check format
  m_impl->srcSurface = get_cairo_surface(*(m_impl->srcBmp));
  m_impl->srcPattern.reset(cairo_pattern_create_for_surface(
    m_impl->srcSurface.get()));
  m_impl->pattern = new Pattern(pattern);
  cairo_pattern_set_extend(m_impl->srcPattern.get(), CAIRO_EXTEND_REPEAT);
  if (m_impl->pattern->GetObjectAligned()){
    cairo_matrix_t matrix = cairo_pattern_matrix_from_tri(m_impl->patternTri,
      floated(pattern.GetAnchor()));
    cairo_pattern_set_matrix(m_impl->srcPattern.get(), &matrix);
  }
  else{
    cairo_matrix_t matrix = translate_matrix(floated(pattern.GetAnchor()));
    cairo_pattern_set_matrix(m_impl->srcPattern, &matrix);
  }
  cairo_set_source(m_impl->cr, m_impl->srcPattern);
}

void CairoContext::set_source_gradient(const Gradient& gradient){
  if (gradient.IsLinear()){
    LinearGradient g(gradient.GetLinear());
    if (m_impl->gradientPattern != nullptr){
      m_impl->gradientPattern.reset(nullptr);
      delete m_impl->linearGradient;
      m_impl->linearGradient = nullptr;
      delete m_impl->radialGradient;
      m_impl->radialGradient = nullptr;
    }

    if (m_impl->srcSurface != nullptr){
      m_impl->srcSurface.reset(nullptr);
      m_impl->srcPattern.reset(nullptr);
      delete m_impl->srcBmp;
      m_impl->srcBmp = nullptr;
    }

    m_impl->linearGradient = new LinearGradient(g);
    m_impl->gradientPattern = faint_cairo_linear_gradient(g);
    cairo_matrix_t matrix = cairo_linear_matrix_from_tri(m_impl->patternTri,
      m_impl->linearGradient->GetAngle(),
      m_impl->linearGradient->GetObjectAligned());
    cairo_pattern_set_matrix(m_impl->gradientPattern, &matrix);
    cairo_set_source(m_impl->cr, m_impl->gradientPattern);
  }
  else if (gradient.IsRadial()){
    RadialGradient g(gradient.GetRadial());
    if (m_impl->gradientPattern != nullptr){
      m_impl->gradientPattern.reset(nullptr);
      delete m_impl->linearGradient;
      m_impl->linearGradient = nullptr;
      delete m_impl->radialGradient;
      m_impl->radialGradient = nullptr;
    }
    if (m_impl->srcSurface != nullptr){
      m_impl->srcSurface.reset(nullptr);
      m_impl->srcPattern.reset(nullptr);
      delete m_impl->srcBmp;
      m_impl->srcBmp = nullptr;
    }
    m_impl->radialGradient = new RadialGradient(g);
    m_impl->gradientPattern = faint_cairo_radial_gradient(g);
    cairo_matrix_t matrix = cairo_radial_matrix_from_tri(m_impl->patternTri,
      *m_impl->radialGradient);
    cairo_pattern_set_matrix(m_impl->gradientPattern, &matrix);
    cairo_set_source(m_impl->cr, m_impl->gradientPattern);
  }
  else{
    assert(false);
  }
}

void CairoContext::set_source_tri(const Tri& t){
  m_impl->patternTri = t;
  if (m_impl->linearGradient != 0){
    cairo_matrix_t matrix = cairo_linear_matrix_from_tri(t,
      m_impl->linearGradient->GetAngle(),
      m_impl->linearGradient->GetObjectAligned());
    cairo_pattern_set_matrix(m_impl->gradientPattern, &matrix);
    cairo_set_source(m_impl->cr, m_impl->gradientPattern);
  }
  else if (m_impl->radialGradient != 0){
    cairo_matrix_t matrix = cairo_radial_matrix_from_tri(t,
      *m_impl->radialGradient);
    cairo_pattern_set_matrix(m_impl->gradientPattern, &matrix);
    cairo_set_source(m_impl->cr, m_impl->gradientPattern);
  }
  else if (m_impl->srcPattern != 0){
    bool objectAligned = m_impl->pattern->GetObjectAligned();
    if (objectAligned){
      cairo_matrix_t matrix = cairo_pattern_matrix_from_tri(t,
        floated(m_impl->pattern->GetAnchor()));
      cairo_pattern_set_matrix(m_impl->srcPattern, &matrix);
    }
    else{
      cairo_matrix_t matrix = translate_matrix(floated(
        m_impl->pattern->GetAnchor()));
      cairo_pattern_set_matrix(m_impl->srcPattern, &matrix);
    }
    cairo_set_source(m_impl->cr, m_impl->srcPattern);
  }
}

void CairoContext::stroke(){
  cairo_stroke(m_impl->cr);
}

void CairoContext::translate(const Point& p){
  cairo_translate(m_impl->cr, p.x, p.y);
}

CairoSave::CairoSave(CairoContext& ctx)
  : m_context(ctx)
{
  m_context.save();
}

CairoSave::~CairoSave(){
  m_context.restore();
}

} // namespace
