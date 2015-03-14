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
#include <cassert>
#include <cmath>
#include <cstddef>
#include "bitmap/alpha-map.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/draw.hh"
#include "geo/arc.hh"
#include "geo/arrowhead.hh"
#include "geo/geo-func.hh"
#include "geo/geo-list-points.hh"
#include "geo/int-rect.hh"
#include "geo/line.hh"
#include "geo/offsat.hh"
#include "geo/pathpt.hh"
#include "geo/points.hh" // Fixme: For tri_from_points, which shouldn't be required here
#include "geo/scale.hh"
#include "geo/tri.hh"
#include "rendering/cairo-context.hh"
#include "rendering/faint-dc.hh"
#include "rendering/filter-class.hh"
#include "text/utf8-string.hh"
#include "util/math-constants.hh"
#include "util/optional.hh"
#include "util/setting-util.hh"
#include "util/settings.hh"

namespace faint{

static Paint get_bg(const Settings& s, const Point& origin){
  Paint bg = get_bg(s);
  return offsat(bg, -floored(origin));
}

static Paint get_bg(const Settings& s, const Point& origin,
  const IntPoint& clickPos)
{
  Paint bg = get_bg(s);
  return offsat(bg, -floored(origin), clickPos);
}

static Paint get_fg(const Settings& s, const Point& origin){
  Paint fg = get_fg(s);
  return offsat(fg, -floored(origin));
}

static Paint get_fg(const Settings& s, const Point& origin,
  const IntPoint& clickPos)
{
  Paint fg = get_fg(s);
  return offsat(fg, -floored(origin), clickPos);
}

static BorderSettings border_settings(const Settings& s, const Paint& p){
  return {p, floored(s.Get(ts_LineWidth)), s.Get(ts_LineStyle)};
}

static BorderSettings border_settings(const Settings& s, const Point& origin){
  return border_settings(s, get_fg(s, origin));
}

static BorderSettings border_settings(const Settings& s, const Point& origin,
  const IntPoint& clickPos)
{
  return border_settings(s, get_fg(s, origin, clickPos));
}

static LineSettings line_settings(const Settings& s, const Paint& p){
  return LineSettings(border_settings(s, p), s.Get(ts_LineCap));
}

static LineSettings line_settings(const Settings& s, const Point& origin){
  return LineSettings(border_settings(s, origin), s.Get(ts_LineCap));
}

// Check if the point is outside the bitmap on the right or bottom sides
bool overextends(const IntPoint& p, const Bitmap& bmp){
  IntSize sz(bmp.GetSize());
  return p.x >= sz.w || p.y >= sz.h;
}

inline void realign_cairo_fill(coord& x, coord& y, coord scale){
  x = static_cast<int>(x * scale + 0.5) / scale;
  y = static_cast<int>(y * scale + 0.5) / scale;
}

Point realign_cairo_fill(const Point& p, coord scale){
  Point p2(p);
  realign_cairo_fill(p2.x, p2.y, scale);
  return p2;
}

inline void realign_cairo_odd_stroke(coord& x, coord& y, coord scale){
  x = (static_cast<int>(x * scale + 0.5) + 0.5) / scale;
  y = (static_cast<int>(y * scale + 0.5) + 0.5) / scale;
}

Point realign_cairo_odd_stroke(const Point& p, coord scale){
  return Point((static_cast<int>(p.x * scale + 0.5) + 0.5) / scale,
    (static_cast<int>(p.y * scale + 0.5) + 0.5) / scale);
}

void realign_cairo(coord& x, coord& y, const Settings& s, coord scale){
  // Cairo aligns fill-operations and strokes with even strokewidths
  // at integer coordinates.  Strokes and fills at half-integer
  // coordinates appear smeared (especially obvious is the case of
  // single pixel horizontal and vertical lines).
  coord lineWidth = s.Get(ts_LineWidth);
  const bool fillOnly = s.Has(ts_FillStyle) &&
    s.Get(ts_FillStyle) == FillStyle::FILL;
  const bool even = int(lineWidth * scale + 0.5) % 2 == 0;
  if (even || fillOnly){
    // Round to nearest integer coordinate for strokes with even linewidth
    // or fills
    realign_cairo_fill(x, y, scale);
  }
  else {
    // Round to nearest .5 for strokes with odd linewidth
    realign_cairo_odd_stroke(x, y, scale);
  }
}

Point realign_cairo(const Point& p, const Settings& s, coord scale){
  coord x = p.x;
  coord y = p.y;
  realign_cairo(x, y, s, scale);
  return Point(x,y);
}

void draw_arrow_head(CairoContext& cr, const Arrowhead& arrowHead,
  coord lineWidth, coord sc)
{
  Settings s;
  s.Set(ts_LineWidth, lineWidth);
  Point p0(realign_cairo(arrowHead.P0(), s, sc));
  Point p1(realign_cairo(arrowHead.P1(), s, sc));
  Point p2(realign_cairo(arrowHead.P2(), s, sc));

  cr.move_to(p0);
  cr.line_to(p1);
  cr.line_to(p2);
  cr.close_path();
  cr.fill();
}

static void fill_and_or_stroke(CairoContext& cr, const Settings& s){
  assert(s.Has(ts_FillStyle));
  cr.set_line_cap(s.GetDefault(ts_LineCap, LineCap::DEFAULT));
  cr.set_line_join(s.GetDefault(ts_LineJoin, LineJoin::DEFAULT));
  coord lineWidth = s.Has(ts_LineWidth) ? s.Get(ts_LineWidth) : 1.0;
  cr.set_line_width(lineWidth);

  if (filled(s)){
    Paint bg(get_bg(s));
    cr.set_source(bg);
    if (border(s)){
      cr.fill_preserve();
    }
    else {
      cr.fill();
      return; // No stroke, return early
    }
  }

  cr.set_source(get_fg(s));
  cr.stroke();
}

static void from_settings(CairoContext& cr, const Settings& s){
  cr.set_line_cap(s.GetDefault(ts_LineCap, LineCap::DEFAULT));
  cr.set_line_join(s.GetDefault(ts_LineJoin, LineJoin::DEFAULT));
  coord lineWidth = s.GetDefault(ts_LineWidth, 1.0);
  cr.set_line_width(lineWidth);

  cr.set_source(get_fg(s)); // Fixme: This might duplicate pattern creation, compare fill_and_or_stroke
  if (dashed(s)) {
    coord dashes[] = {2 * lineWidth, 2 * lineWidth};
    cr.set_dash(dashes, 2, 0);
  }
  else {
    cr.set_dash(0, 0, 0);
  }
}

static bool has_front_arrow(const Settings& s){
  return s.Has(ts_LineArrowhead) && s.
    Get(ts_LineArrowhead) == LineArrowhead::FRONT;
}

static std::vector<Point> transform_points(const std::vector<Point>& points,
  coord scale, const Point& origin)
{
  std::vector<Point> v2;
  v2.reserve(points.size());
  for (const Point& pt : points){
    v2.push_back(pt * scale + origin);
  }
  return v2;
}

static void rounded_rectangle(CairoContext& cr, const Tri& tri, const Settings& s){
  from_settings(cr, s);
  cr.set_source_tri(tri);

  CairoSave cairoSave(cr);
  cr.translate(tri.P0());
  cr.rotate(tri.GetAngle());

  const auto width = tri.Width();
  const auto height = tri.Height();
  auto radius = std::min({
    s.GetDefault(ts_RadiusX, 0.0),
    width / 2,
    height / 2});

  cr.arc({width - radius, radius},
    radius,
    {-90_deg, 0_deg});

  cr.arc({width - radius, height - radius},
    radius,
    {0_deg, 90_deg});

  cr.arc({radius, height - radius},
    radius,
    {90_deg, 180_deg});

  cr.arc({radius, radius},
    radius,
    {180_deg, 270_deg});

  cr.close_path();
  fill_and_or_stroke(cr, s);
}

FaintDC::FaintDC(Bitmap& bmp)
  : FaintDC(bmp, origin_t(Point(0,0)), 1.0)
{}

FaintDC::FaintDC(Bitmap& bmp, const origin_t& origin, coord scale)
  : m_bitmap(bmp),
    m_cr(new CairoContext(m_bitmap)),
    m_origin(0,0),
    m_sc(1.0)
{
  SetOrigin(origin.Get());
  SetScale(scale);
}

FaintDC::~FaintDC(){
  // Implementation required for releasing unique_ptrs for types that
  // are merely forward declared in the header.
}

void FaintDC::Arc(const Tri& tri, const AngleSpan& span, const Settings& s){
  if (!anti_aliasing(s)){
    return;
  }

  m_cr->set_source_tri(tri);
  from_settings(*m_cr, s);
  std::vector<Point> v = arc_as_path(tri, span);
  Point c(center_point(tri));
  m_cr->move_to(c);
  m_cr->line_to(v[0]);
  for (size_t i = 1; i < v.size(); i+= 3){
    m_cr->curve_to(v[i], v[i+1], v[i+2]);
  }
  m_cr->close_path();
  fill_and_or_stroke(*m_cr, s);
}

void FaintDC::Blit(const Bitmap& bmp, const Point& topLeft,
  const Settings& settings)
{
  IntPoint imagePt(floored(topLeft * m_sc + m_origin));
  if (overextends(imagePt, m_bitmap)){
    return;
  }
  bool alphaBlend = alpha_blending(settings);
  BackgroundStyle bgStyle = settings.Get(ts_BackgroundStyle);

  Paint bg(settings.Get(ts_Bg));
  if (bg.IsColor() && bgStyle  == BackgroundStyle::MASKED){
    Color bgCol(bg.GetColor());
    if (alphaBlend){
      BitmapBlendAlphaMasked(bmp, bgCol, imagePt);
    }
    else{
      BitmapSetAlphaMasked(bmp, bgCol, imagePt);
    }
  }
  else {
    if (alphaBlend){
      BitmapBlendAlpha(bmp, imagePt);
    }
    else {
      BitmapSetAlpha(bmp, imagePt);
    }
  }
}

void FaintDC::BitmapBlendAlpha(const Bitmap& drawnBitmap,
  const IntPoint& topLeft)
{
  if (m_sc < 1){
    const Bitmap scaled(scale_bilinear(drawnBitmap, Scale(m_sc)));
    blend(offsat(scaled, topLeft), onto(m_bitmap));
  }
  else if (m_sc > 1){
    const Bitmap scaled(scale_nearest(drawnBitmap, truncated(m_sc)));
    blend(offsat(scaled, topLeft), onto(m_bitmap));
  }
  else {
    blend(offsat(drawnBitmap, topLeft), onto(m_bitmap));
  }
}

void FaintDC::BitmapBlendAlphaMasked(const Bitmap& drawnBitmap,
  const Color& maskColor, const IntPoint& topLeft)
{
  if (m_sc < 1){
    const Bitmap scaled(scale_bilinear(drawnBitmap, Scale(m_sc)));
    blend_masked(offsat(scaled, topLeft), onto(m_bitmap), maskColor);
  }
  else if (m_sc > 1){
    const Bitmap scaled(scale_nearest(drawnBitmap, truncated(m_sc)));
    blend_masked(offsat(scaled, topLeft), onto(m_bitmap), maskColor);
  }
  else {
    blend_masked(offsat(drawnBitmap, topLeft), onto(m_bitmap), maskColor);
  }
}

void FaintDC::BitmapSetAlpha(const Bitmap& drawnBitmap,
  const IntPoint& topLeft)
{
  if (m_sc < 1){
    const Bitmap scaled(scale_bilinear(drawnBitmap, Scale(m_sc)));
    blit(offsat(scaled, topLeft), onto(m_bitmap));
  }
  else if (m_sc > 1){
    const Bitmap scaled(scale_nearest(drawnBitmap, truncated(m_sc)));
    blit(offsat(scaled, topLeft), onto(m_bitmap));
  }
  else {
    blit(offsat(drawnBitmap, topLeft), onto(m_bitmap));
  }
}

void FaintDC::BitmapSetAlphaMasked(const Bitmap& drawnBitmap,
  const Color& maskColor, const IntPoint& topLeft)
{
  if (m_sc < 1){
    Bitmap scaled(scale_bilinear(drawnBitmap, Scale(m_sc)));
    blit_masked(offsat(scaled, topLeft), onto(m_bitmap), maskColor);
  }
  else if (m_sc > 1){
    Bitmap scaled(scale_nearest(drawnBitmap, truncated(m_sc)));
    blit_masked(offsat(scaled, topLeft), onto(m_bitmap), maskColor);
  }
  else {
    blit_masked(offsat(drawnBitmap, topLeft), onto(m_bitmap), maskColor);
  }
}

void FaintDC::Blend(const AlphaMap& alpha, const IntPoint& topLeft,
  const IntPoint& anchor, const Settings& s)
{
  IntPoint imagePt(floored(floated(topLeft) * m_sc + m_origin));
  if (overextends(imagePt, m_bitmap)){
    return;
  }

  Filter* f = get_filter(s);
  if (f != nullptr){
    // Fixme: Need to offset the alpha-map and such.
    Padding p(f->GetPadding());
    Bitmap bmp(alpha.GetSize() + p.GetSize(), color_transparent_white);
    IntPoint offset(p.left, p.top);
    blend(offsat(alpha.FullReference(), imagePt), onto(bmp),
      get_fg(s, m_origin, anchor));
    f->Apply(bmp);
    blend(offsat(bmp, imagePt - offset), onto(m_bitmap));
    delete f;
  }
  else{
    IntRect r(floored(-m_origin), m_bitmap.GetSize());
    IntSize alphaSz(alpha.GetSize());
    if (r.x + r.w > alphaSz.w || r.y + r.h >= alphaSz.h){
      blend(offsat(alpha.FullReference(), imagePt),
        onto(m_bitmap), get_fg(s, m_origin, anchor));
    }
    else{
      blend(offsat(alpha.SubReference(r), imagePt + r.TopLeft()),
        onto(m_bitmap), get_fg(s, m_origin, anchor));
    }
  }
}

void FaintDC::Clear(const Color& color){
  clear(m_bitmap, color);
}

void FaintDC::DrawRasterEllipse(const Tri& tri, const Settings& s){
  Filter* f = get_filter(s);
  if (f != nullptr){
    DrawRasterEllipse(tri, *f, s);
    delete f;
    return;
  }
  IntRect r(floored(tri.P0() * m_sc + m_origin),
    floored(tri.P3() * m_sc + m_origin));

  if (filled(s)){
    fill_ellipse(m_bitmap, r, get_bg(s, m_origin, rounded(center_point(tri))));
  }
  if (border(s)){
    draw_ellipse(m_bitmap, r, border_settings(s, m_origin));
  }
}

void FaintDC::DrawRasterEllipse(const Tri& tri, const Filter& f,
  const Settings& s)
{
  IntRect r(floored(tri.P0() * m_sc + m_origin),
    floored(tri.P3() * m_sc + m_origin));

  Padding p(f.GetPadding());
  Bitmap bmp(r.GetSize() + p.GetSize(), color_transparent_white);
  IntPoint offset(p.left, p.top);
  IntRect r2(offset, r.GetSize());
  if (filled(s)){
    fill_ellipse(bmp, r2,
      get_bg(s, m_origin + floated(offset - r.TopLeft()),
        rounded(center_point(tri))));
  }
  if (border(s)){
    draw_ellipse(bmp, r2,
      border_settings(s, m_origin + floated(offset - r.TopLeft())));
  }
  f.Apply(bmp);
  BitmapBlendAlpha(bmp, floored((tri.P0() - floated(offset)) * m_sc +
    m_origin));
}

void FaintDC::DrawRasterPolygon(const std::vector<Point>& points,
  const Settings& s)
{
  if (points.size() < 2){
    return;
  }
  std::vector<IntPoint> points2(floored(transform_points(points, m_sc,
    m_origin)));

  if (filled(s)){
    fill_polygon(m_bitmap, points2, get_bg(s, m_origin));
  }
  if (border(s)){
    draw_polygon(m_bitmap, points2, border_settings(s, m_origin));
  }
}

void FaintDC::DrawRasterPolyLine(const std::vector<Point>& points,
  const Settings& s)
{
  assert(points.size() >= 2);
  bool skipLast = points.size() > 2 && points[points.size() - 2] ==
    points[points.size() -1];
  std::vector<Point> points2(transform_points(points, m_sc, m_origin));
  if (skipLast){
    // Remove the last point if the two last points are identical
    // to avoid random arrowhead-direction for this case.
    points2.pop_back();
  }
  coord lineWidth = s.Get(ts_LineWidth);
  Paint paint(get_fg(s, m_origin));
  std::vector<IntPoint> points3(floored(points2));

  if (has_front_arrow(s)){
    Point p0 = points2[points2.size() - 2];
    const Point& p1 = points2.back();
    Arrowhead a(get_arrowhead(LineSegment(p0, p1), lineWidth));
    points3.back() = rounded(a.LineAnchor());
    fill_triangle(m_bitmap, rounded(a.P0()), rounded(a.P1()), rounded(a.P2()),
      paint);
  }
  draw_polyline(m_bitmap,
    points3,
    line_settings(s, paint));
}

void FaintDC::DrawRasterRect(const Tri& tri, const Settings& s){
  Filter* f = get_filter(s);
  if (f != 0){
    DrawRasterRect(tri, *f, s);
    delete f;
    return;
  }
  IntRect r(floored(tri.P0() * m_sc + m_origin),
    floored(tri.P3() * m_sc + m_origin));

  Optional<Paint> fill;
  if (filled(s)){
    fill.Set(get_bg(s, m_origin));
  }
  Optional<BorderSettings> outline;
  if (border(s)){
    outline.Set(border_settings(s, m_origin, rounded(center_point(tri))));
  }

  rect(m_bitmap, r, outline, fill);
}

void FaintDC::DrawRasterRect(const Tri& tri, const Filter& f,
  const Settings& s)
{
  IntRect r(floored(tri.P0() * m_sc + m_origin),
    floored(tri.P3() * m_sc + m_origin));

  Padding p(f.GetPadding());
  Bitmap bmp(r.GetSize() + p.GetSize(), color_transparent_white);
  IntPoint offset(p.left, p.top);
  IntRect r2(offset, r.GetSize());

  Optional<Paint> fill;
  if (filled(s)){
    fill.Set(get_bg(s, m_origin));
  }
  Optional<BorderSettings> outline;
  if (border(s)){
    outline.Set(border_settings(s, m_origin, rounded(center_point(tri))));
  }
  rect(bmp, r2, outline, fill);
  f.Apply(bmp);
  BitmapBlendAlpha(bmp, floored((tri.P0() - floated(offset)) * m_sc +
    m_origin));
}

void FaintDC::Ellipse(const Tri& tri, const Settings& s){
  if (!anti_aliasing(s)){
    DrawRasterEllipse(tri, s);
    return;
  }
  Path(ellipse_as_path(tri), s);
}

Color FaintDC::GetPixel(const Point& posView) const{
  IntPoint posBmp(floored(posView * m_sc + m_origin));
  assert(point_in_bitmap(m_bitmap, posBmp));
  return get_color(m_bitmap, posBmp);
}

std::vector<PathPt> FaintDC::GetTextPath(const Tri& t, const utf8_string& text,
  const Settings& s)
{
  return m_cr->get_text_path(t, text, s);
}

bool FaintDC::IsOk() const{
  return m_cr->IsOk();
}

bool FaintDC::IsTargetting(const Bitmap& bmp) const{
  return m_cr->IsTargetting(bmp);
}

std::string FaintDC::ErrorString() const{
  return m_cr->ErrorString();
}

void FaintDC::Line(const LineSegment& line, const Filter& f, const Settings& s){
  // Fixme: Raster only (filter variant)
  IntSize extraSize(40,40);
  // Fixme: use bounding-rect function for line.
  Bitmap bmp(truncated(Size(std::fabs(line.p1.x - line.p0.x),
    std::fabs(line.p1.y - line.p0.y))) + extraSize,
    color_transparent_white);
  IntPoint origin = floored(min_coords(line.p0, line.p1));
  IntPoint offset(10,10);
  draw_line(bmp, {floored(line.p0) - origin + offset,
    floored(line.p1) - origin + offset}, line_settings(s, m_origin));
  f.Apply(bmp);
  BitmapBlendAlpha(bmp, floored(floated(origin - offset) * m_sc + m_origin));
}

void FaintDC::Line(const LineSegment& line, const Settings& s){
  const std::vector<Point> pts = {line.p0, line.p1};
  PolyLine(tri_from_points(pts), pts, s);
}

struct CairoArc{
  Point center;
  Radii r;
  coord startAngleRad;
  coord angleExtentRad;
  coord xAxisRotationRad;
};

// Based on
// http://commons.oreilly.com/wiki/index.php/SVG_Essentials/Paths#Elliptical_Arc
// and
// http://www.w3.org/TR/SVG/implnote.html#ArcImplementationNotes
static CairoArc cairo_arc_from_svg_arc(const Point& p0,
  Radii r,
  coord xAxisRotation,
  int largeArcFlag,
  int sweepFlag,
  const Point& p)
{

  // Step 1: Compute x1', y1' (i.e. p1)
  const Point d = (p0 - p) / 2.0;
  xAxisRotation = std::fmod(xAxisRotation, 2 * math::pi);
  coord cosAngle = std::cos(xAxisRotation);
  coord sinAngle = std::sin(xAxisRotation);

  const Point p1(cosAngle * d.x + sinAngle * d.y,
    -sinAngle * d.x + cosAngle * d.y);

  // F6.6 Correction of out of range radii
  r = abs(r); // F.6.6.1
  const Point p1sq = p1 * p1;
  Radii rsq = r * r;

  // Fixme: Ensure non-zero radius
  coord radiusCheck = p1sq.x / rsq.x  + p1sq.y / rsq.y; // F.6.6.2
  if (radiusCheck > 1){
    r *= sqrt(radiusCheck); // F.6.6.3
    rsq = r * r;
  }

  // Step 2: Compute cx', cy' (i.e. c1)
  coord sign = (largeArcFlag == sweepFlag) ? - 1 : 1;
  coord sq = ((rsq.x*rsq.y) - (rsq.x*p1sq.y) - (rsq.y*p1sq.x)) /
    ((rsq.x*p1sq.y) + (rsq.y*p1sq.x));
  sq = (sq < 0) ? 0 : sq;
  coord coeff = sign * sqrt(sq);
  Point c1(coeff * ((r.x * p1.y) / r.y),
    coeff * -((r.y * p1.x) / r.x));

  // Step 3: Compute cx, cy from cx', cy' (i.e. c from c1)
  Point sp2 = (p0 + p) / 2.0;
  Point c(sp2.x + (cosAngle * c1.x - sinAngle * c1.y),
    sp2.y + (sinAngle * c1.x + cosAngle * c1.y));

  // Step 4: Compute theta and delta (i.e. angle and angle extent)
  coord ux = (p1.x - c1.x) / r.x;
  coord uy = (p1.y - c1.y) / r.y;
  coord vx = (-p1.x - c1.x) / r.x;
  coord vy = (-p1.y - c1.y) / r.y;
  coord n = sqrt(ux * ux + uy * uy);
  coord pp = ux; // 1 * ux + 0 * uy
  sign = (uy < 0) ? -1 : 1;

  coord theta = sign * acos(pp / n);

  n = sqrt((ux * ux + uy * uy) * (vx * vx + vy * vy));
  pp = ux * vx + uy * vy;
  sign = ((ux * vy - uy * vx) < 0) ? -1 : 1;
  coord delta = sign * acos(pp / n);

  if (sweepFlag == 0 && delta > 0){
    delta -= 2 * math::pi;
  }
  else if (sweepFlag == 1 && delta < 0){
    delta += 2 * math::pi;
  }
  delta = fmod(delta, 2*math::pi);
  CairoArc result;
  result.center = c;
  result.r = r;
  result.startAngleRad = theta;
  result.angleExtentRad = delta;
  result.xAxisRotationRad = xAxisRotation;
  return result;
}

void FaintDC::Path(const std::vector<PathPt>& points, const Settings& s){
  if (points.empty()){
    return;
  }

  from_settings(*m_cr, s);
  Point currPos = points.front().p;

  Tri patternTri(tri_from_points(points)); // Fixme: Pass tri instead
  if (!rather_zero(patternTri)){
    m_cr->set_source_tri(patternTri);
  }

  for (const PathPt& pt : points){
    if (pt.IsMove()){
      m_cr->move_to(pt.p);
      currPos = pt.p;
    }
    else if (pt.IsLine()){
      m_cr->line_to(pt.p);
      currPos = pt.p;
    }
    else if (pt.IsCubicBezier()){
      m_cr->curve_to(pt.c, pt.d, pt.p);
      currPos = pt.p;
    }
    else if (pt.ClosesPath()){
      m_cr->close_path();
    }
    else if (pt.IsArc()){
      CairoSave cairoSave(*m_cr);
      CairoArc arc = cairo_arc_from_svg_arc(currPos,
        pt.r,
        pt.axisRotation.Rad(),
        pt.largeArcFlag,
        pt.sweepFlag,
        pt.p);
      m_cr->translate(arc.center);
      m_cr->rotate(pt.axisRotation);

      coord angle1 = arc.startAngleRad;
      coord angle2 = arc.startAngleRad + arc.angleExtentRad;

      m_cr->scale(arc.r.x, arc.r.y);
      if (angle1 < angle2){
        m_cr->arc(Point(0,0), 1.0, AngleSpan::Rad(angle1, angle2));
      }
      else{
        m_cr->arc_negative(Point(0,0), 1.0, AngleSpan::Rad(angle1, angle2));
      }
      currPos = pt.p;
    }
  }
  fill_and_or_stroke(*m_cr, s);
}

void FaintDC::PenStroke(const std::vector<IntPoint>& points, const Settings& s){
  if (points.empty()){
    return;
  }

  const Paint fg = get_fg(s, m_origin);
  if (points.size() == 1){
    const IntPoint p(floored(points[0] * m_sc + m_origin));
    const int width = 1;
    draw_line(m_bitmap, {p, p}, {fg, width, LineStyle::SOLID, LineCap::BUTT});
    return;
  }

  IntPoint prev = floored(floated(points[0]) * m_sc + m_origin);
  for (size_t i = 1; i != points.size(); i++){
    const IntPoint pt = floored(points[i] * m_sc + m_origin);
    draw_line(m_bitmap, {prev, pt}, {fg,
      truncated(m_sc), // ?
      LineStyle::SOLID, LineCap::BUTT});
    prev = pt;
  }
}

void FaintDC::PolyLine(const Tri& tri, const std::vector<Point>& points,
  const Settings& s)
{
  if (points.empty()){
    return;
  }

  if (!anti_aliasing(s)){
    DrawRasterPolyLine(points, s);
    return;
  }

  from_settings(*m_cr, s);
  m_cr->set_source_tri(tri);

  // Move to the first point
  const Point p0(realign_cairo(points[0], s, m_sc));
  m_cr->move_to(p0);

  // Add line segments to all consecutive points except (normally-)
  // the last to allow adjustment for arrowhead. If the two last
  // points are identical, only add lines up to the last two points,
  // to avoid adding a line segment that extends over the arrow head.
  bool skipTwo = points.size() > 2 && points[points.size() - 1] ==
    points[points.size() - 2];
  size_t firstEnd = skipTwo ? points.size() - 2 : points.size() - 1;
  // Add line segments to all consecutive points except the last
  for (size_t i = 0; i != firstEnd; i++){
    const Point pt(realign_cairo(points[i], s, m_sc));
    m_cr->line_to(pt);
  }

  // The source point for the last line segment.  If the last two
  // points are identical, use the pen-penultimate point as source to
  // avoid random arrowhead direction.
  const Point from(skipTwo ? points[points.size() - 3] :
    points[points.size() - 2]);
  const Point to(points.back());
  if (has_front_arrow(s)){
    Arrowhead a(get_arrowhead(LineSegment(from, to), s.Get(ts_LineWidth)));
    Point anchor = realign_cairo(a.LineAnchor(), s, m_sc);
    m_cr->line_to(anchor);
    m_cr->stroke();
    draw_arrow_head(*m_cr, a, s.Get(ts_LineWidth), m_sc);
  }
  else {
    Point p(realign_cairo(to, s, m_sc));
    m_cr->line_to(p);
    m_cr->stroke();
  }
}

void FaintDC::Polygon(const Tri& tri, const std::vector<Point>& points,
  const Settings& s)
{
  if (points.size() <= 1){
    return;
  }

  if (!anti_aliasing(s)){
    DrawRasterPolygon(points, s);
    return;
  }

  from_settings(*m_cr, s);
  const Point& p0 = points[0];
  coord x(p0.x);
  coord y(p0.y);
  realign_cairo(x, y, s, m_sc);
  m_cr->set_source_tri(tri);
  m_cr->move_to(Point(x, y)); // Fixme
  for (size_t i = 1; i != points.size(); i++){
    const Point& p(points[i]);
    x = p.x;
    y = p.y;
    realign_cairo(x, y, s, m_sc);
    m_cr->line_to(Point(x, y));
  }
  m_cr->close_path();
  fill_and_or_stroke(*m_cr, s);
}

void FaintDC::Rectangle(const Tri& tri, const Settings& s){
  if (!anti_aliasing(s)){
    DrawRasterRect(tri, s);
    return;
  }

  const bool rounded = s.GetDefault(ts_RadiusX, 0.0) != 0.0;
  if (rounded){
    rounded_rectangle(*m_cr, tri, s);
  }
  else{
    auto points(points_clockwise(tri)); // Fixme: Weird, why copying, then iterate?
    Polygon(tri, std::vector<Point>(points.begin(), points.end()), s);
  }
}

void FaintDC::SetOrigin(const Point& origin){
  m_origin = origin;
  m_cr->translate(origin);
}

void FaintDC::SetScale(coord scale){
  m_sc = scale;
  m_cr->scale(scale, scale);
}

void FaintDC::Spline(const std::vector<Point>& points, const Settings& s){
  if (points.size() <= 2){
    return;
  }

  from_settings(*m_cr, s);
  const Point& p0 = points[0];
  coord x1 = p0.x;
  coord y1 = p0.y;

  const Point& p2 = points[1];
  coord c = p2.x;
  coord d = p2.y;

  coord x3 = (x1 + c) / 2;
  coord y3 = (y1 + d) / 2;

  m_cr->move_to(Point(x1, y1));
  m_cr->line_to(Point(x3, y3));

  for (size_t i = 2; i < points.size(); i++){
    const Point& pt = points[i];
    x1 = x3;
    y1 = y3;
    const coord x2 = c;
    const coord y2 = d;
    c = pt.x;
    d = pt.y;
    x3 = (x2 + c) / 2;
    y3 = (y2 + d) / 2;

    m_cr->curve_to(Point(x1, y1),
      Point(x2, y2),
      Point(x3, y3));
  }
  m_cr->line_to(Point(c,d));
  m_cr->stroke();
}

void FaintDC::Text(const Tri& t, const utf8_string& text, const Settings& s){
  m_cr->set_source_tri(t);
  m_cr->pango_text(t, text, s);
}

void FaintDC::Text(const Tri& t, const utf8_string& text, const Settings& s,
  const Tri& clip)
{
  CairoSave save(*m_cr);
  m_cr->set_source_tri(t);
  m_cr->set_clip_polygon(points_clockwise(clip));
  m_cr->pango_text(t, text, s);
}

void FaintDC::Text(const Tri& t, const utf8_string& text, const Settings& s,
  const Optional<Tri>& maybeClip)
{
  maybeClip.Visit(
    [&](const Tri& clip){
      Text(t, text, s, clip);
    },
    [&](){
      Text(t, text, s);
    });
}

Size FaintDC::TextSize(const utf8_string& text, const Settings& s) const{
  return m_cr->pango_text_size(text, s);
}

TextMeasures FaintDC::TextExtents(const utf8_string& text,
  const Settings& s) const
{
  // Fixme: Maybe this (and pango_text_extents) not needed.
  return m_cr->pango_text_extents(text, s);
}

FontMetrics FaintDC::GetFontMetrics(const Settings& s) const{
  return m_cr->pango_font_metrics(s);
}

std::vector<int> FaintDC::CumulativeTextWidth(const utf8_string& text,
  const Settings& s) const
{
  return m_cr->cumulative_text_width(text, s);
}

} // namespace
