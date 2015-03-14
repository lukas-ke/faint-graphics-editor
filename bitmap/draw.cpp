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
#include <unordered_set>
#include <set> // Fixme: Remove if using unordered_set all over.
#include "bitmap/alpha-map.hh"
#include "bitmap/auto-crop.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/bitmap-templates.hh"
#include "bitmap/color.hh"
#include "bitmap/draw.hh"
#include "bitmap/pattern.hh"
#include "geo/axis.hh"
#include "geo/geo-func.hh"
#include "geo/geo-list-points.hh"
#include "geo/int-size.hh"
#include "geo/line.hh"
#include "geo/measure.hh"
#include "geo/offsat.hh"
#include "geo/point.hh"
#include "geo/range.hh"
#include "geo/scale.hh"
#include "geo/size.hh"
#include "util/common-fwd.hh" // Fixme: For axis
#include "util/optional.hh"

namespace faint{

BorderSettings::BorderSettings(const LineSettings& s)
  : paint(s.paint), lineWidth(s.lineWidth), lineStyle(s.lineStyle)
{}

LineSettings solid_1px(const Color& c){
  return LineSettings(c, 1, LineStyle::SOLID, LineCap::BUTT);
}

LineSettings with_long_dash(LineSettings l){
  l.lineStyle = LineStyle::LONG_DASH;
  return l;
}

bool intersects(const Offsat<Bitmap>& src, const DstBmp& dst){
  IntPoint p0 = src.Offset();
  IntSize dstSz(dst.GetSize());
  IntSize srcSz(src->GetSize());

  if (p0.x >= dstSz.w || p0.y >= dstSz.h){
    return false;
  }
  if (p0.x + srcSz.w <= 0 || p0.y + srcSz.h <= 0){
    return false;
  }
  return true;
}

bool intersects(const Offsat<AlphaMapRef>& alphaMap, const Bitmap& dst){
  // alphaOffset is the offset of the alphaMap's top-left corner
  // relative to the bitmap.
  IntPoint dstSz = point_from_size(dst.GetSize());
  IntPoint maxPoint = point_from_size(alphaMap->GetSize()) + alphaMap.Offset();
  return fully_positive(maxPoint) &&
    fully_positive(dstSz - alphaMap.Offset());
}

bool intersects(const Offsat<AlphaMapRef>& alphaMap, const DstBmp& dst){
  return intersects(alphaMap, dst.Get());
}

class BorderAttributes{
public:
  BorderAttributes(const BorderSettings& s)
    : lineWidth(s.lineWidth), lineStyle(s.lineStyle)
  {}

  BorderAttributes(int lineWidth, LineStyle lineStyle)
    : lineWidth(lineWidth),
      lineStyle(lineStyle)
  {}
  int lineWidth;
  LineStyle lineStyle;
};

class LineAttributes{
public:
  LineAttributes(const LineSettings& s)
    : lineWidth(s.lineWidth), lineStyle(s.lineStyle), cap(s.cap)
  {}
  LineAttributes(int lineWidth, LineStyle style, LineCap cap)
    : lineWidth(lineWidth), lineStyle(style), cap(cap)
  {}

  int lineWidth;
  LineStyle lineStyle;
  LineCap cap;
};

LineAttributes with_cap(const LineAttributes& l, LineCap cap){
  LineAttributes l2(l);
  l2.cap = cap;
  return l2;
}

LineAttributes with_cap(const BorderAttributes& b, LineCap cap){
  return LineAttributes(b.lineWidth, b.lineStyle, cap);
}

using std::swap;

#define EVER ;;

class color_ptr{
public:
  // For fetching rgba values from an uchar-pointer
  // (Which must be at the correct four-byte offset)
  uchar& r;
  uchar& g;
  uchar& b;
  uchar& a;
  color_ptr(uchar* p):
    r(*(p + iR)),
    g(*(p + iG)),
    b(*(p + iB)),
    a(*(p + iA))
  {}
  void Set(const Color& c){
    r = c.r;
    g = c.g;
    b = c.b;
    a = c.a;
  }
  color_ptr& operator=(const color_ptr&) = delete;
};

class const_color_ptr{
public:
  // For fetching rgba values from an uchar-pointer
  // (Which must be at the correct four-byte offset)
  const uchar r;
  const uchar g;
  const uchar b;
  const uchar a;
  const_color_ptr(const uchar* p):
    r(*(p + iR)),
    g(*(p + iG)),
    b(*(p + iB)),
    a(*(p + iA))
  {}

  const_color_ptr& operator=(const const_color_ptr&) = delete;
};

bool operator==(const color_ptr& c1, const Color& c2){
  return c1.r == c2.r &&
    c1.g == c2.g &&
    c1.b == c2.b &&
    c1.a == c2.a;
}

static int int_abs(int v){
  return v < 0 ? -v : v;
}

static IntPoint get_offset(int x0, int y0, int x1, int y1, int lineWidth){
  const bool steep = int_abs(y1 - y0) > int_abs(x1 - x0);
  if (steep){
    swap(x0,y0);
    swap(x1,y1);
  }
  if (x0 > x1){ // Not implemented
    swap(x0, x1);
    swap(y0,y1);
  }
  int dx = x1 - x0;
  int dy = int_abs(y1 - y0);
  int p = 2 * dy - int_abs(dx);
  int yStep = y0 < y1 ? -1 : 1;
  int at_x = 0;
  int at_y = 0;
  int yp = 0;

  coord distance = 0;
  coord diag = sqrt(2.0);
  for (int xp = 0; xp != lineWidth; xp++){
    at_x = - yp;
    at_y = - xp;
    if (p < 0){
      p += 2*dy;
      distance += 1;
    }
    else {
      yp += yStep;
      p += 2*dy - 2*dx;
      distance += diag;
    }
    if (distance >= lineWidth){
      break;
    }
  }
  if (steep){
    return IntPoint(at_y, at_x);
  }
  return IntPoint(at_x, at_y);
}

static std::map<int,int> ellipse_points(int x0, int y0, int a, int b){
  if (a == 0 || b == 0){
    return std::map<int,int>();
  }

  std::map<int,int> points;
  int a2 = 2 * a * a;
  int b2 = 2 * b * b;
  int error = a * a * b;
  int x = 0;
  int y = b;
  int stopy = 0;
  int stopx = a2 * b;

  while (stopy <= stopx) {
    //put_pixel(bmp, x0 + x, y0 + y, c);
    points[ y0 + y ] = x0 - x;
    points[ y0 - y ] = x0 - x;
    //put_pixel(bmp, x0 + x, y0 - y, c);
    x++;

    error -= b2 * (x - 1);
    stopy += b2;
    if (error <= 0) {
      error += a2 * (y - 1);
      y--;
      stopx -= a2;
    }
  }

  error = b*b*a;
  x = a;
  y = 0;
  stopy = b2*a;
  stopx = 0;
  while (stopy >= stopx) {

    //put_pixel(bmp, x0 + x, y0 + y, c);
    points[ y0 + y ] = x0 - x;
    points[ y0 - y ] = x0 - x;
    // put_pixel(bmp, x0 + x, y0 - y, c);
    y++;
    error -= a2 * (y - 1);
    stopx += a2;
    if (error < 0) {
      error += b2 * (x - 1);
      x--;
      stopy -= b2;
    }
  }
  return points;
}

template<typename Functor>
void scanline_f(Bitmap& bmp, int x0, int x1, int y, const Functor& setPixFunc){
  if (x0 > x1){
    swap(x0,x1);
  }
  for (int x = x0; x <= x1; x++){
    setPixFunc(bmp, x, y);
  }
}

template<typename Functor>
void scanline_fl_f(Bitmap& bmp, coord x0, coord x1, coord y, const Functor& setPixFunc){
  scanline_f(bmp, floored(x0), floored(x1), floored(y), setPixFunc);
}

template<typename Functor>
void vertical_f(Bitmap& bmp, int y0, int y1, int x, const Functor& setPixFunc){
  if (y0 > y1){
    swap(y0,y1);
  }
  for (int y = y0; y <= y1; y++){
    setPixFunc(bmp, x, y);
  }
}

template<typename Functor>
void line_circle_f(Bitmap& bmp, const Functor& setPixFunc, const IntPoint& p,
  int lineWidth)
{
  int halfLw = (lineWidth - 1) / 2;
  const IntPoint e0(-halfLw, -halfLw);
  const IntPoint e1(halfLw, halfLw);
  fill_ellipse_f(bmp, setPixFunc, IntRect(p + e0, p + e1));
}

template<typename Functor>
void draw_wide_line_f(Bitmap& bmp, const IntLineSegment& line,
  const Functor& setPixFunc, const LineAttributes& s)
{
  int x0 = line.p0.x;
  int y0 = line.p0.y;
  int x1 = line.p1.x;
  int y1 = line.p1.y;
  const bool steep = int_abs(y1 - y0) > int_abs(x1 - x0);
  if (steep){
    swap(x0,y0);
    swap(x1,y1);
  }
  if (x0 > x1){
    swap(x0, x1);
    swap(y0,y1);
  }

  const int dx = x1 - x0;
  const int dy = int_abs(y1 - y0);
  const int yStep = y0 < y1 ? -1 : 1;
  int p = 2 * dy - int_abs(dx); // Error for the line
  int pp_o = p; // Outer-error for the parallel line (p-line)

  // Use a bounding-line as kind of dumb method to bound the p-lines
  // so that the opposite edge stays non-jagged, and steps in the
  // same manner as the start line
  const IntPoint pt = get_offset(x0,y0, x1, y1, s.lineWidth);
  int xMin = x0 + pt.x * yStep; // x-boundary for the p-lines
  int yMin = y0 + pt.y * yStep; // y-boundary for the p-lines
  int p2 = p; // Error for the boundary

  // The p-lines are shifted to the left for each "jump" in the p-line
  // due to jumps in the regular line
  int xOff = 0;
  int y = 0; // Jumps in the line

  // Loop along the regular line
  if (dx == 0){
    return;
  }
  bool on = false;
  const bool dashes = s.lineStyle != LineStyle::SOLID;
  int steps = 0;
  for (int i = 0; i - 1 != dx - xOff; i++){
    if (steps == 0){
      on = !dashes || !on;
    }
    steps = (steps + 1) % (s.lineWidth * 2);
    int pp = pp_o; // Offset the error for this p-line for this "height"
    int yp = 0; // Jumps in the p-line
    int xStart = x0 + i + xOff;
    int yStart = y0 - y;
    int at_x;
    int at_y;

    // For each i, draw a parallel line
    int xp = 0;
    for (EVER) {
      at_x = xStart - yp * yStep;
      at_y = yStart - xp * yStep;
      if (at_x < xMin){
        // Break without setting a pixel if about to jump across
        // a finished segment
        break;
      }
      assert(xp <= s.lineWidth);
      if (steep){
        if (on){
          setPixFunc(bmp, at_y - yStep * pt.y / 2, at_x - yStep * pt.x / 2);
        }
      }
      else{
        if (on){
          setPixFunc(bmp, at_x - yStep * pt.x / 2, at_y - yStep * pt.y / 2);
        }
      }
      if ((yStep == 1 && at_y <= yMin) || (yStep == -1 && at_y >= yMin)){
        // Break after setting the final pixel
        break;
      }
      if (pp < 0){
        pp += 2*dy;
      }
      else {
        yp += yStep;
        pp += 2*dy - 2*dx;
      }
      xp++;
    }

    // Update the bounding line when it's been hit
    if (at_x == xMin && at_y == yMin){
      xMin += 1;
      if (p2 < 0){
        p2 += 2 * dy;
      }
      else {
        yMin -= yStep;
        p2 += 2 * dy - 2*dx;
      }
    }

    // Update the start line
    if (p < 0){
      p += 2*dy;
    }
    else {
      y += yStep;
      p += 2*dy - 2*dx;
      if (pp_o < 0){
        pp_o += 2*dy;
      }
      else {
        pp_o += 2*dy - 2*dx;
        p -= 2 * dy;
        xOff -= 1;
      }
    }
  }

  if (s.cap == LineCap::ROUND){
    line_circle_f(bmp, setPixFunc, line.p0, s.lineWidth);
    line_circle_f(bmp, setPixFunc, line.p1, s.lineWidth);
  }
}

template<typename Functor>
void draw_line_f(Bitmap& bmp, const Functor& setPixFunc,
  const IntLineSegment& line, const LineAttributes& s)
{
  int lineWidth = std::max(1, s.lineWidth);
  if (lineWidth > 1){
    draw_wide_line_f(bmp, line, setPixFunc, s);
    return;
  }

  int x0 = line.p0.x;
  int y0 = line.p0.y;
  int x1 = line.p1.x;
  int y1 = line.p1.y;
  const bool steep = int_abs(y1 - y0) > int_abs(x1 - x0);
  if (steep){
    swap(x0, y0);
    swap(x1, y1);
  }
  if (x0 > x1){
    swap(x0, x1);
    swap(y0, y1);
  }

  int dx = x1 - x0;
  int dy = int_abs(y1 - y0);
  coord err = 0;
  int y = y0;
  int yStep = y0 < y1 ? 1 : -1;

  bool on = false;
  int steps = 0;

  const bool dashes = s.lineStyle != LineStyle::SOLID;

  for (int x = x0; x<= x1; x++){
    if (steps == 0){
      on = !dashes || !on;
    }
    steps = (steps + 1) % 2;

    err += dy;
    if (steep){
      if (on){
        setPixFunc(bmp, y, x);
      }
    }
    else {
      if (on){
        setPixFunc(bmp, x, y);
      }
    }
    if (2 * err > dx){
      y += yStep;
      err -= dx;
    }
  }
}

template<typename Functor>
void draw_wide_ellipse_f(Bitmap& bmp, const Functor& setPixFunc,
  const IntRect& r, const BorderAttributes& s)
{
  IntPoint radiuses(floored(floated(IntPoint(r.w, r.h)) / 2.0));
  int x0 = r.x + radiuses.x;
  int y0 = r.y + radiuses.y;
  int a = radiuses.x;
  int b = radiuses.y;

  if (a == 0){
    // \ref(wide-ellipse-dash), use s.lineStyle later
    draw_line_f(bmp, setPixFunc, {r.TopLeft(), r.BottomLeft()},
      LineAttributes(s.lineWidth, LineStyle::SOLID, LineCap::BUTT));
    return;
  }
  if (b == 0){
    // \ref(wide-ellipse-dash), use s.dashes later
    draw_line_f(bmp, setPixFunc, {r.TopLeft(), r.TopRight()},
      LineAttributes(s.lineWidth, LineStyle::SOLID, LineCap::BUTT));
    return;
  }

  std::map<int,int> points = ellipse_points(x0, y0,
    a - s.lineWidth, b - s.lineWidth);
  for (auto ptPair : points){
    setPixFunc(bmp, ptPair.second, ptPair.first);
  }

  int a2 = 2 * a * a;
  int b2 = 2 * b * b;
  int error = a * a * b;

  int x = 0;
  int y = b;

  int xoffset = r.w % 2 == 0 ? 1 : 0;
  int yoffset = r.h % 2 == 0 ? 1 : 0;

  int stopy = 0;
  int stopx = a2 * b;

  // \def(wide-ellipse-dash) Fixme: support dash;
  while (stopy <= stopx) {
    setPixFunc(bmp, x0 + x - xoffset, y0 + y - yoffset);

    if (points.find(y0 + y) != points.end()){
      int x1 = points[y0 + y];
      scanline_f(bmp, x0 - x, x1, y0 + y - yoffset, setPixFunc);
      int dx = x1 - (x0 - x);
      scanline_f(bmp, x0 + x - dx - xoffset,
        x0 + x - xoffset, y0 + y - yoffset, setPixFunc);
    }
    else {
      scanline_f(bmp, x0 - x, x0 + x - xoffset,
        y0 + y - yoffset, setPixFunc);
    }

    if (points.find(y0 - y) != points.end()){
      int x1 = points[y0 - y];
      scanline_f(bmp, x0 - x, x1, y0 - y, setPixFunc);
      int dx = x1 - (x0 - x);
      scanline_f(bmp, x0 + x - xoffset - dx, x0 + x - xoffset,
        y0 - y, setPixFunc);
    }
    else {
      scanline_f(bmp, x0 - x, x0 + x - xoffset, y0 - y, setPixFunc);
    }
    x++;

    error -= b2 * (x - 1);
    stopy += b2;
    if (error <= 0) {
      error += a2 * (y - 1);
      y--;
      stopx -= a2;
    }
  }

  error = b*b*a;
  x = a;
  y = 0;
  stopy = b2*a;
  stopx = 0;
  while (stopy >= stopx) {
    if (points.find(y0 + y) != points.end()){
      int x1 = points[y0 + y];
      scanline_f(bmp, x0 - x, x1, y0 + y - yoffset, setPixFunc);
      int dx = x1 - (x0 - x);
      scanline_f(bmp, x0 + x - xoffset - dx, x0 + x - xoffset,
        y0 + y - yoffset, setPixFunc);
    }
    else {
      scanline_f(bmp, x0 - x, x0 + x - xoffset, y0 + y - yoffset, setPixFunc);
    }

    if (points.find(y0 - y) != points.end()){
      int x1 = points[y0 - y];
      scanline_f(bmp, x0 - x, x1, y0 - y, setPixFunc);
      int dx = x1 - (x0 - x);
      scanline_f(bmp, x0 + x - xoffset - dx, x0 + x - xoffset,
        y0 - y, setPixFunc);
    }
    else {
      scanline_f(bmp, x0 - x, x0 + x - xoffset, y0 - y, setPixFunc);
    }
    y++;
    error -= a2 * (y - 1);
    stopx += a2;
    if (error < 0) {
      error += b2 * (x - 1);
      x--;
      stopy -= b2;
    }
  }
}

template<typename Functor>
void draw_thin_ellipse_f(Bitmap& bmp,
  const Functor& setPixFunc,
  const IntRect& r,
  LineStyle lineStyle)
{
  IntPoint radiuses(floored(floated(IntPoint(r.w, r.h)) / 2.0));
  int x0 = r.x + radiuses.x;
  int y0 = r.y + radiuses.y;
  int a = radiuses.x;
  int b = radiuses.y;

  if (a == 0){
    draw_line_f(bmp, setPixFunc, left_side(r),
      LineAttributes(1, lineStyle, LineCap::BUTT));
    return;
  }
  if (b == 0){
    draw_line_f(bmp, setPixFunc, top_side(r),
      LineAttributes(1, lineStyle, LineCap::BUTT));
    return;
  }

  int a2 = 2 * a * a;
  int b2 = 2 * b * b;
  int error = a * a * b;
  int x = 0;
  int y = b;

  int stopy = 0;
  int stopx = a2 * b;

  int xoffset = r.w % 2 == 0 ? 1 : 0;
  int yoffset = r.h % 2 == 0 ? 1 : 0;
  bool on = false;
  const bool dashes = lineStyle != LineStyle::SOLID;
  int steps = 0;
  while (stopy <= stopx) {
    if (steps == 0){
      on = !dashes || !on;
    }
    steps = (steps + 1) % 2;
    if (on){

      setPixFunc(bmp, x0 + x - xoffset, y0 + y - yoffset);
      setPixFunc(bmp, x0 - x, y0 + y - yoffset);
      setPixFunc(bmp, x0 - x, y0 - y);
      setPixFunc(bmp, x0 + x - xoffset, y0 - y);
    }
    x++;

    error -= b2 * (x - 1);
    stopy += b2;
    if (error <= 0) {
      error += a2 * (y - 1);
      y--;
      stopx -= a2;
    }
  }

  error = b*b*a;
  x = a;
  y = 0;
  stopy = b2*a;
  stopx = 0;
  while (stopy >= stopx) {
    if (steps == 0){
      on = !dashes || !on;
    }
    steps = (steps + 1) % 2;
    if (on){
      setPixFunc(bmp, x0 + x - xoffset, y0 + y - yoffset);
      setPixFunc(bmp, x0 - x, y0 + y - yoffset);
      setPixFunc(bmp, x0 - x, y0 - y);
      setPixFunc(bmp, x0 + x - xoffset, y0 - y);
    }
    y++;
    error -= a2 * (y - 1);
    stopx += a2;
    if (error < 0) {
      error += b2 * (x - 1);
      x--;
      stopy -= b2;
    }
  }
}
template<typename Functor>
void draw_ellipse_f(Bitmap& bmp, const Functor& setPixFunc, const IntRect& r,
  const BorderAttributes& s)
{
  if (s.lineWidth > 1){
    draw_wide_ellipse_f(bmp, setPixFunc, r, s);
  }
  else{
    draw_thin_ellipse_f(bmp, setPixFunc, r, s.lineStyle);
  }
}

template<typename Functor>
void draw_polygon_f(Bitmap& bmp,
  const Functor& setPixFunc,
  const std::vector<IntPoint>& points,
  const BorderAttributes& s)
{
  if (points.empty()){
    return;
  }
  if (points.size() == 1){
    const IntPoint& p(points[0]);
    draw_line_f(bmp, setPixFunc, {p, p},
      with_cap(s, LineCap::ROUND));
  }

  for (size_t i = 1; i != points.size(); i++){
    draw_line_f(bmp, setPixFunc, {points[ i - 1], points[i]},
      with_cap(s, LineCap::ROUND));
    line_circle_f(bmp, setPixFunc, points[i], s.lineWidth);
  }
  draw_line_f(bmp, setPixFunc, {points.back(), points[0]},
    with_cap(s, LineCap::BUTT));
  line_circle_f(bmp, setPixFunc, points[0], s.lineWidth);
}

template<typename Functor>
void draw_polyline_f(Bitmap& bmp,
  const Functor& setPixFunc,
  const std::vector<IntPoint>& points,
  const LineAttributes& s)
{
  if (points.empty()){
    return;
  }
  if (points.size() == 1){
    const IntPoint& p(points[0]);
    draw_line_f(bmp, setPixFunc, {p, p}, s);
  }

  for (size_t i = 1; i != points.size(); i++){
    draw_line_f(bmp, setPixFunc, {points[i - 1], points[i]},
      with_cap(s, LineCap::BUTT));

    if (i != points.size() - 1){
      line_circle_f(bmp, setPixFunc, points[i], s.lineWidth);
    }
  }
  if (s.cap == LineCap::ROUND){
    line_circle_f(bmp, setPixFunc, points[0], s.lineWidth);
    line_circle_f(bmp, setPixFunc, points.back(), s.lineWidth);
  }
}

template<typename Functor>
void draw_rect_f(Bitmap& bmp,
  const Functor& setPixFunc,
  const IntRect& r,
  const BorderAttributes& s)
{
  const bool dashed = s.lineStyle != LineStyle::SOLID;
  if (!dashed){
    int lineWidth = std::max(1, s.lineWidth);
    for (int i = 0; i != lineWidth; i++){
      scanline_f(bmp, r.Left(), r.Right(), r.Top() + i, setPixFunc);
      scanline_f(bmp, r.Left(), r.Right(), r.Bottom() - i, setPixFunc);
      vertical_f(bmp, r.Top(), r.Bottom(), r.Left() + i, setPixFunc);
      vertical_f(bmp, r.Top(), r.Bottom(), r.Right() - i, setPixFunc);
    }
  }
  else {
    draw_polygon_f(bmp, setPixFunc, as_polygon(r), s);
  }
}

template<typename Functor>
void fill_polygon_f(Bitmap& bmp, const Functor& setPixFunc,
  const std::vector<IntPoint>& inPoints)
{
  if (inPoints.empty()){
    return;
  }
  IntRect r = bounding_rect(inPoints);
  std::vector<IntPoint> points(inPoints);
  points.push_back(points[0]);
  int minX = r.Left() - 1; // Fixme
  int maxX = std::min(r.Right(), bmp.m_w - 1);
  int minY = std::max(0, r.Top());
  int maxY = std::min(r.Bottom(), bmp.m_h - 1);
  for (int y = minY; y <= maxY; y++){
    std::vector<int> x_vals;
    for (size_t i = 0; i < points.size() - 1; i+= 1){
      IntPoint p0 = points[i];
      IntPoint p1 = points[i + 1];
      int x0 = p0.x;
      int x1 = p1.x;
      int y0 = p0.y;
      int y1 = p1.y;
      if (x0 > x1){
        swap(x0,x1);
        swap(y0,y1);
      }
      if ((y0 < y && y <= y1) || (y1 < y && y <= y0)){
        if (x0 == x1){
          x_vals.push_back(x0);
          continue;
        }
        coord k = coord(y1 - y0) / (x1 - x0);
        coord m = y0 - k * x0;
        int x = static_cast<int>((y - m) / k);
        x_vals.push_back(x);
      }
    }
    if (x_vals.empty()){
      continue;
    }
    std::sort(begin(x_vals), end(x_vals));
    for (int x = minX; x <= maxX; x++){
      for (size_t j = 0; j != x_vals.size(); j++){
        if (x < x_vals[j]){
          if ((x_vals.size() - j) % 2 != 0) {
            setPixFunc(bmp, x + 1, y); // Fixme: Why + 1? (Added to fill the insides of polygon edge in gradient panel)
          }
          break;
        }
      }
    }
  }
}

template<typename Functor>
void fill_rect_f(Bitmap& bmp, const Functor& setPixFunc, const IntRect& r){
  const int x0 = r.x;
  const int y0 = r.y;
  const int x1 = r.Right();
  const int y1 = r.Bottom();

  for (int y = y0; y <= y1; y++){
    for (int x = x0; x <= x1; x++){
      setPixFunc(bmp, x, y);
    }
  }
}

template<typename Functor>
void fill_triangle_f(Bitmap& bmp,
  const Functor& setPixFunc,
  const IntPoint& p0,
  const IntPoint& p1,
  const IntPoint& p2)
{
  std::vector<IntPoint> v = {p0, p1, p2};
  fill_polygon_f(bmp, setPixFunc, v);
  draw_polygon_f(bmp, setPixFunc, v, {1, LineStyle::SOLID});
}

static Bitmap flip_horizontal(const Bitmap& src){
  Bitmap dst(src);
  uchar* pDst = dst.m_data;
  uchar* pSrc = src.m_data;
  for (int y = 0; y != src.m_h; y++){
    for (int x = 0 ; x != src.m_w; x++){
      int iSrc = y * src.m_row_stride + x * BPP;
      int iDst = y * dst.m_row_stride + (dst.m_w - 1) * BPP - x * BPP;
      pDst[ iDst + iR ] = pSrc[iSrc + iR ];
      pDst[ iDst + iG ] = pSrc[iSrc + iG ];
      pDst[ iDst + iB ] = pSrc[iSrc + iB ];
      pDst[ iDst + iA ] = pSrc[iSrc + iA ];
    }
  }
  return dst;
}

static Bitmap flip_vertical(const Bitmap& src){
  Bitmap dst(src);
  uchar* pDst = dst.m_data;
  uchar* pSrc = src.m_data;
  for (int y = 0; y != src.m_h; y++){
    for (int x = 0 ; x != src.m_w; x++){
      int iSrc = y * src.m_row_stride + x * BPP;
      int iDst = (dst.m_h - y - 1)* dst.m_row_stride + x * BPP;
      pDst[ iDst + iR ] = pSrc[iSrc + iR ];
      pDst[ iDst + iG ] = pSrc[iSrc + iG ];
      pDst[ iDst + iB ] = pSrc[iSrc + iB ];
      pDst[ iDst + iA ] = pSrc[iSrc + iA ];
    }
  }
  return dst;
}

struct IsColor{
  IsColor(const Bitmap& bmp, const Color& c)
    : m_bmp(bmp),
      m_color(c)
  {}
  bool operator()(int x, int y) const{
    return get_color_raw(m_bmp, x, y) == m_color;
  }
  const Bitmap& m_bmp;
  const Color m_color;

  IsColor& operator=(const IsColor&) = delete;
};

// This macro is used to dispatch calls to the correct
// Bitmap-modifying template function depending on the content of a
// Paint.  The macro must be used in a scope with a Paint
// named "paint" and a Bitmap named "bmp".
//
// The macro arguments are:
//  1) The unqualified name of the template function
//  2) expression yielding a ColorFromColor
//  3) expression yielding a ColorFromPattern
//  4) expression yielding a ColorFromGradient
//  5+) additional arguments for the dispatched to function.
//
// The dispatched to function must be a template function, on this format:
// template<typename Functor>
// void function(Bitmap&, const Functor&, .. additional arguments..).
//
// Implementation note: the explicit template specification used
// within the macro (e.g. func<ColorFromColor>) makes confused order
// of functors in the macro call a compile time error - otherwise this
// would cause a runtime crash for an attempt to instantiate for
// example a ColorFromPattern with a Paint containing a gradient.
#define DISPATCH(func, PAINT, COLORFUNC, PATTERNFUNC, GRADIENTFUNC, ...) \
if  (PAINT.IsColor()){ \
  func<ColorFromColor>(bmp,COLORFUNC, __VA_ARGS__); \
} \
else if (PAINT.IsPattern()){\
  func<ColorFromPattern>(bmp, PATTERNFUNC, __VA_ARGS__); \
}\
else if (PAINT.IsGradient()){\
  func<ColorFromGradient>(bmp, GRADIENTFUNC, __VA_ARGS__); \
}\
else{ \
  assert(false); \
}
// End of DISPATCH macro

Bitmap alpha_blended(const Bitmap& src, const Color& color){
  Bitmap dst(src.GetSize(), color);
  blend(at_top_left(src), onto(dst));
  return dst;
}

Bitmap alpha_blended(const Bitmap& src, const ColRGB& c){
  return alpha_blended(src, Color(c, 255));
}

void blend_alpha(Bitmap& bmp, const ColRGB& color){
  bmp = alpha_blended(bmp, color);
}

void blend(const Offsat<Bitmap>& src, DstBmp dst){
  if (!intersects(src, dst)){
    return;
  }

  const int x0 = src.Offset().x;
  const int y0 = src.Offset().y;

  const int xMin = std::max(0, -x0);
  const int yMin = std::max(0, -y0);
  IntSize dstSz(dst.GetSize());
  const int xMax = std::min(dstSz.w - x0, src->m_w);
  const int yMax = std::min(dstSz.h - y0, src->m_h);

  const int srcStride = src->GetStride();
  const int dstStride = dst.GetStride();
  const uchar* srcData = src->GetRaw();
  uchar* dstData = dst.GetRaw();
  for (int y = yMin; y != yMax; y++){
    for (int x = xMin; x != xMax; x++){
      int srcPos = y * srcStride + x * BPP;
      int dstPos = (y + y0) * dstStride + (x + x0) * BPP;
      float alpha = srcData[srcPos + iA];
      dstData[dstPos + iR] = static_cast<uchar>((srcData[srcPos + iR] * alpha +
        dstData[dstPos + iR] * (255 - alpha)) / 255);
      dstData[dstPos + iG] = static_cast<uchar>((srcData[srcPos + iG] * alpha +
        dstData[dstPos + iG] * (255 - alpha)) / 255);
      dstData[dstPos + iB] = static_cast<uchar>((srcData[srcPos + iB] * alpha +
        dstData[dstPos + iB] * (255 - alpha)) / 255);
      dstData[dstPos + iA] =
        std::max(srcData[srcPos + iA], dstData[dstPos + iA]);
    }
  }
}

void blend_masked(const Offsat<Bitmap>& src, DstBmp dst,
  const Color& maskColor)
{
  if (!intersects(src, dst)){
    return;
  }

  int x0 = src.Offset().x;
  int y0 = src.Offset().y;

  IntSize dstSz(dst.GetSize());
  const int xMin = std::max(0, -x0);
  const int yMin = std::max(0, -y0);
  const int xMax = std::min(dstSz.w - x0, src->m_w);
  const int yMax = std::min(dstSz.h - y0, src->m_h);

  int srcStride = src->GetStride();
  int dstStride = dst.GetStride();
  const uchar* srcData = src->GetRaw();
  uchar* dstData = dst.GetRaw();

  for (int y = yMin; y != yMax; y++){
    for (int x = xMin; x != xMax; x++){
      int srcPos = y * srcStride + x * BPP;
       if (srcData[srcPos + iA] == 0 ||
         (srcData[ srcPos + iA ] == maskColor.a &&
           srcData[ srcPos + iR ] == maskColor.r &&
           srcData[ srcPos + iG ] == maskColor.g &&
           srcData[ srcPos + iB ] == maskColor.b)) {
         continue;
       }

      int dstPos = (y + y0) * dstStride + (x + x0) * BPP;
      uchar alpha = srcData[srcPos + iA];
      dstData[dstPos + iR] = static_cast<uchar>((srcData[srcPos + iR] * alpha +
         dstData[dstPos + iR] * (255 - alpha)) / 255);
      dstData[dstPos + iG] = static_cast<uchar>((srcData[srcPos + iG] * alpha +
         dstData[dstPos + iG] * (255 - alpha)) / 255);
      dstData[dstPos + iB] = static_cast<uchar>((srcData[srcPos + iB] * alpha +
         dstData[dstPos + iB] * (255 - alpha)) / 255);
    }
  }
}

static void blend_pattern(const Offsat<AlphaMapRef>& inAlphaMap, DstBmp dst,
  const Pattern& p)
{
  const AlphaMapRef& alphaMap = inAlphaMap.Get();
  auto blendFunc = [&](int x, int y){return alphaMap.Get(x,y);};

  Bitmap strokeMap(alphaMap.GetSize()); // Fixme: Costly allocation
  blend_pixels(strokeMap, ColorFromPattern(p), blendFunc);
  blend(offsat(strokeMap, inAlphaMap.Offset()), dst);
}

static void blend_gradient(const Offsat<AlphaMapRef>& offsatAlphaMap,
  DstBmp dst,
  const Gradient& g)
{
  const AlphaMapRef& alphaMap = offsatAlphaMap.Get();
  alphaMap.BoundingRect().Visit(
    [&](const IntRect& r){
      Bitmap strokeMap(alphaMap.GetSize()); // Fixme: Costly allocation
      blend_pixels(strokeMap, ColorFromGradient(g, r),
        [&](int x, int y){
          return alphaMap.Get(x,y);});
      blend(offsat(strokeMap, offsatAlphaMap.Offset()), dst);
    });
}

static void blend_color(const Offsat<AlphaMapRef>& offsatAlphaMap, DstBmp dst,
  const Color& c)
{
  const AlphaMapRef& alphaMap(offsatAlphaMap.Get());
  const IntPoint topLeft(offsatAlphaMap.Offset());

  const int xMin = std::max(0, -topLeft.x);
  const int yMin = std::max(0, -topLeft.y);
  IntSize srcSz(alphaMap.GetSize());
  IntSize dstSz(dst.GetSize());
  const int xMax = std::min(dstSz.w - topLeft.x, srcSz.w);
  const int yMax = std::min(dstSz.h - topLeft.y, srcSz.h);

  const int stride = dst.GetStride();
  uchar* dstData = dst.GetRaw();

  for (int y = yMin; y != yMax; y++){
    for (int x = xMin; x != xMax; x++){
      int dstPos = (y + topLeft.y) * stride + (x + topLeft.x) * BPP;
      uchar alpha = alphaMap.Get(x,y);
      dstData[dstPos + iR] = static_cast<uchar>((c.r * alpha +
          dstData[dstPos + iR] * (255 - alpha)) / 255);
      dstData[dstPos + iG] = static_cast<uchar>((c.g * alpha +
          dstData[dstPos + iG] * (255 - alpha)) / 255);
      dstData[dstPos + iB] = static_cast<uchar>((c.b * alpha +
          dstData[dstPos + iB] * (255 - alpha)) / 255);
      dstData[dstPos + iA] = static_cast<uchar>((c.a * alpha +
          dstData[dstPos + iA] * (255 - alpha)) / 255);
    }
  }
}

void blend(const Offsat<AlphaMapRef>& alphaMap, DstBmp dst, const Paint& paint){
  if (!intersects(alphaMap, dst)){
    return;
  }
  visit(paint,
    [&](const Color& c){
      blend_color(alphaMap, dst, c);
    },
    [&](const Pattern& p){
      blend_pattern(alphaMap, dst, offsat(p, alphaMap.Offset()));
    },
    [&](const Gradient& g){
      blend_gradient(alphaMap, dst, g);
    });
}

void blit(const Offsat<Bitmap>& src, DstBmp dst){
  if (!intersects(src, dst)){
    return;
  }

  int x0 = src.Offset().x;
  int y0 = src.Offset().y;

  IntSize dstSz(dst.GetSize());
  const int xMin = std::max(0, -x0);
  const int yMin = std::max(0, -y0);
  const int xMax = std::min(dstSz.w - x0, src->m_w);
  const int yMax = std::min(dstSz.h - y0, src->m_h);
  const int dstStride = dst.GetStride();
  const int srcStride = src->GetStride();
  uchar* dstData = dst.GetRaw();
  const uchar* srcData = src->GetRaw();

  for (int y = yMin; y != yMax; y++){
    for (int x = xMin; x != xMax; x++){
      int srcPos = y * srcStride + x * BPP;
      int dstPos = (y + y0) * dstStride + (x + x0) * BPP;
      dstData[ dstPos ] = srcData[srcPos ];
      dstData[ dstPos + 1 ] = srcData[srcPos + 1 ];
      dstData[ dstPos + 2 ] = srcData[srcPos + 2 ];
      dstData[ dstPos + 3 ] = srcData[srcPos + 3 ];
    }
  }
}

void blit_masked(const Offsat<Bitmap>& src, DstBmp dst, const Color& maskColor){
  if (!intersects(src, dst)){
    return;
  }

  int x0 = src.Offset().x;
  int y0 = src.Offset().y;
  const int xMin = std::max(0, -x0);
  const int yMin = std::max(0, -y0);

  IntSize dstSz(dst.GetSize());
  const int xMax = std::min(dstSz.w - x0, src->m_w);
  const int yMax = std::min(dstSz.h - y0, src->m_h);

  const int srcStride = src->GetStride();
  const int dstStride = dst.GetStride();

  uchar* dstData = dst.GetRaw();
  const uchar* srcData = src->GetRaw();

  for (int y = yMin; y != yMax; y++){
    for (int x = xMin; x != xMax; x++){
      int srcPos = y * srcStride + x * BPP;
      int dstPos = (y + y0) * dstStride + (x + x0) * BPP;
      if (
          srcData[ srcPos + 3 ] != maskColor.a ||
          srcData[ srcPos + 2 ] != maskColor.r ||
          srcData[ srcPos + 1 ] != maskColor.g ||
          srcData[ srcPos ] != maskColor.b)
        {
          dstData[ dstPos ] = srcData[srcPos ];
          dstData[ dstPos + 1 ] = srcData[srcPos + 1 ];
          dstData[ dstPos + 2 ] = srcData[srcPos + 2 ];
          dstData[ dstPos + 3 ] = srcData[srcPos + 3 ];
        }
    }
  }
}

static void boundary_fill_color(Bitmap& bmp, const IntPoint& pos,
  const Color& fillColor, const Color& boundaryColor)
{
  if (get_color(bmp, pos) == boundaryColor){
    return;
  }
  std::vector<IntPoint> v;
  std::set<IntPoint> used; // Fixme: Consider unordered_set
  v.push_back(pos);

  for (uint i = 0; i != v.size(); i++){
    IntPoint w = v[i];
    bool addedUp = false;
    bool addedDown = false;
    for (EVER){
      put_pixel(bmp, w, fillColor);
      used.insert(w);
      IntPoint above = w - delta_y(1);
      if (above.y > 0 && used.find(above) == used.end() &&
        get_color(bmp, above) != boundaryColor)
      {
        if (!addedUp){
          addedUp = true;
          v.push_back(above);
        }
      }
      else{
        addedUp = false;
      }
      IntPoint below = w + delta_y(1);
      if (below.y != bmp.m_h && used.find(below) == used.end() &&
        get_color(bmp, below) != boundaryColor)
      {
        if (!addedDown){
          v.push_back(below);
          addedDown = true;
        }
      }
      else{
        addedDown = false;
      }

      IntPoint nextW = w - delta_x(1);
      if (nextW.x < 0 || used.find(nextW) != used.end() ||
        get_color(bmp, nextW) == boundaryColor)
      {
        break;
      }
      w = nextW;
    }

    addedUp = false;
    addedDown = false;
    IntPoint e = v[i];
    for (EVER){
      put_pixel(bmp, e, fillColor);
      used.insert(e);
      IntPoint nextE = e + delta_x(1);
      if (nextE.x == bmp.m_w || used.find(nextE) != used.end() ||
        get_color(bmp, nextE) == boundaryColor)
      {
        break;
      }
      e = nextE;
      IntPoint above = e - delta_y(1);
      if (above.y >= 0 && used.find(above) == used.end() &&
        get_color(bmp, above) != boundaryColor)
      {
        if (!addedUp){
          addedUp = true;
          v.push_back(above);
        }
      }
      else{
        addedUp = false;
      }

      IntPoint below = e + delta_y(1);
      if (below.y != bmp.m_h && used.find(below) == used.end() &&
        get_color(bmp, below) != boundaryColor)
      {
        if (!addedDown){
          v.push_back(below);
          addedDown = true;
        }
      }
      else{
        addedDown = false;
      }
    }
  }
}

void boundary_fill_pattern_relative(Bitmap& bmp, const IntPoint& pos,
  const Pattern& pattern, const Color& boundaryColor)
{
  if (get_color(bmp, pos) == boundaryColor){
    return;
  }

  Bitmap filled(bmp);
  std::vector<IntPoint> v;
  std::set<IntPoint> used; // Fixme: Consider unordered_set
  v.push_back(pos);

  const Bitmap& patBmp(pattern.GetBitmap());
  const IntPoint patOffset = pattern.GetAnchor();

  for (uint i = 0; i != v.size(); i++){
    IntPoint w = v[i];
    bool addedUp = false;
    bool addedDown = false;
    for (EVER){
      put_pixel(filled, w, get_color_modulo(patBmp, w + patOffset));
      used.insert(w);
      IntPoint above = w - delta_y(1);
      if (above.y > 0 && used.find(above) == used.end() &&
        get_color(bmp, above) != boundaryColor)
      {
        if (!addedUp){
          addedUp = true;
          v.push_back(above);
        }
      }
      else{
        addedUp = false;
      }
      IntPoint below = w + delta_y(1);
      if (below.y != bmp.m_h && used.find(below) == used.end() &&
        get_color(bmp, below) != boundaryColor)
      {
        if (!addedDown){
          v.push_back(below);
          addedDown = true;
        }
      }
      else{
        addedDown = false;
      }

      IntPoint nextW = w - delta_x(1);
      if (nextW.x < 0 || used.find(nextW) != used.end() ||
        get_color(bmp, nextW) == boundaryColor)
      {
        break;
      }
      w = nextW;
    }

    addedUp = false;
    addedDown = false;
    IntPoint e = v[i];
    for (EVER){
      put_pixel(filled, e, get_color_modulo(patBmp, e + patOffset));
      used.insert(e);
      IntPoint nextE = e + delta_x(1);
      if (nextE.x == bmp.m_w || used.find(nextE) != used.end() ||
        get_color(bmp, nextE) == boundaryColor)
      {
        break;
      }
      e = nextE;
      IntPoint above = e - delta_y(1);
      if (above.y >= 0 && used.find(above) == used.end() &&
        get_color(bmp, above) != boundaryColor)
      {
        if (!addedUp){
          v.push_back(above);
          addedUp = true;
        }
      }
      else{
        addedUp = false;
      }
      IntPoint below = e + delta_y(1);
      if (below.y != bmp.m_h && used.find(below) == used.end() &&
        get_color(bmp, below) != boundaryColor)
      {
        if (!addedDown){
          v.push_back(below);
          addedDown = true;
        }
      }
      else{
        addedDown = false;
      }
    }
  }
  blit(at_top_left(filled), onto(bmp));
}

void boundary_fill_gradient(Bitmap& bmp, const IntPoint& pos,
  const Gradient& gradient, const Color& boundaryColor)
{
  if (get_color(bmp, pos) == boundaryColor){
    return;
  }

  Color fillColor = boundaryColor == color_black ? color_white : color_black;
  Bitmap filled(bmp.GetSize(), fillColor == color_white ? color_black :
    color_white);

  std::vector<IntPoint> v;
  std::set<IntPoint> used; // Fixme: Consider unordered_set
  v.push_back(pos);

  // Min and max filled pixels, used to determine the gradient rectangle
  int min_x = bmp.m_w - 1;
  int min_y = bmp.m_h - 1;
  int max_x = 0;
  int max_y = 0;

  for (uint i = 0; i != v.size(); i++){
    IntPoint w = v[i];
    bool addedUp = false;
    bool addedDown = false;
    for (EVER){
      put_pixel(filled, w, fillColor);
      put_pixel(bmp, w, fillColor);
      min_x = std::min(min_x, w.x);
      max_x = std::max(max_x, w.x);
      min_y = std::min(min_y, w.y);
      max_y = std::max(max_y, w.y);

      used.insert(w);
      IntPoint above = w - delta_y(1);
      if (above.y > 0 && used.find(above) == used.end() &&
        get_color(bmp, above) != boundaryColor)
      {
        if (!addedUp){
          v.push_back(above);
          addedUp = true;
        }
      }
      else{
        addedUp = false;
      }
      IntPoint below = w + delta_y(1);
      if (below.y != bmp.m_h && used.find(below) == used.end() &&
        get_color(bmp, below) != boundaryColor)
      {
        if (!addedDown){
          v.push_back(below);
        }
      }
      else{
        addedDown = false;
      }

      IntPoint nextW = w - delta_x(1);
      if (nextW.x < 0 || used.find(nextW) != used.end() ||
        get_color(bmp, nextW) == boundaryColor)
      {
        break;
      }
      w = nextW;
    }

    addedUp = false;
    addedDown = false;
    IntPoint e = v[i];
    for (EVER){
      put_pixel(bmp, e, fillColor);
      put_pixel(filled, e, fillColor);
      min_x = std::min(min_x, e.x);
      max_x = std::max(max_x, e.x);
      min_y = std::min(min_y, e.y);
      max_y = std::max(max_y, e.y);

      used.insert(e);
      IntPoint nextE = e + delta_x(1);
      if (nextE.x == bmp.m_w || used.find(nextE) != used.end() ||
        get_color(bmp, nextE) == boundaryColor)
      {
        break;
      }
      e = nextE;
      IntPoint above = e - delta_y(1);
      if (above.y >= 0 && used.find(above) == used.end() &&
        get_color(bmp, above) != boundaryColor)
      {
        if (!addedUp){
          v.push_back(above);
          addedUp = true;
        }
      }
      else{
        addedUp = false;
      }
      IntPoint below = e + delta_y(1);
      if (below.y != bmp.m_h && used.find(below) == used.end() &&
        get_color(bmp, below) != boundaryColor)
      {
        if (!addedDown){
          v.push_back(below);
          addedDown = true;
        }
      }
      else{
        addedDown = false;
      }
    }
  }

  Bitmap grBmp(IntSize(max_x - min_x + 1, max_y - min_y + 1), Paint(gradient));
  set_alpha(grBmp, 255);
  for (int y = 0; y != grBmp.m_h; y++){
    for (int x = 0; x != grBmp.m_w; x++){
      if (get_color_raw(filled, x + min_x, y + min_y) != fillColor){
        put_pixel_raw(grBmp, x, y, color_transparent_white);
      }
    }
  }
  blend(offsat(grBmp, min_x, min_y), onto(bmp));
}

void boundary_fill(Bitmap& bmp, const IntPoint& pos, const Paint& fillPaint,
  const Color& boundaryColor)
{
  visit(fillPaint,
    [&](const Color& c){
      boundary_fill_color(bmp, pos, c, boundaryColor);
    },
    [&](const Pattern& p){
      boundary_fill_pattern_relative(bmp, pos, p, boundaryColor);
    },
    [&](const Gradient& g){
      boundary_fill_gradient(bmp, pos, g, boundaryColor);
    });
}

void draw_ellipse(Bitmap& bmp, const IntRect& r, const BorderSettings& s){
  DISPATCH(draw_ellipse_f, s.paint,
    ColorFromColor(s.paint.GetColor()),
    ColorFromPattern(s.paint.GetPattern()),
    ColorFromGradient(s.paint.GetGradient(), r),
    r, s);
}

void draw_line(Bitmap& bmp, const IntLineSegment& line, const LineSettings& s){
  DISPATCH(draw_line_f, s.paint,
    ColorFromColor(s.paint.GetColor()),
    ColorFromPattern(s.paint.GetPattern()),
    ColorFromGradient(s.paint.GetGradient(), inflated(bounding_rect(line),
      s.lineWidth)), // Fixme: This should be aligned with the line
    line, s);
}

void draw_hline(Bitmap& bmp, int y, const std::pair<int, int>& x,
  const LineSettings& s)
{
  draw_line(bmp, {{x.first, y}, {x.second, y}}, s);
}

void draw_vline(Bitmap& bmp, int x, const std::pair<int, int>& y,
  const LineSettings& s)
{
  draw_line(bmp, {{x, y.first}, {x, y.second}}, s);
}

void draw_polygon(Bitmap& bmp, const std::vector<IntPoint>& points,
  const BorderSettings& s)
{
  DISPATCH(draw_polygon_f, s.paint,
    ColorFromColor(s.paint.GetColor()),
    ColorFromPattern(s.paint.GetPattern()),
    ColorFromGradient(s.paint.GetGradient(), inflated(bounding_rect(points),
      s.lineWidth)),
    points, s);
}

void draw_polyline(Bitmap& bmp, const std::vector<IntPoint>& points, const
  LineSettings& s)
{
  DISPATCH(draw_polyline_f, s.paint,
    ColorFromColor(s.paint.GetColor()),
    ColorFromPattern(s.paint.GetPattern()),
    ColorFromGradient(s.paint.GetGradient(), inflated(bounding_rect(points),
      s.lineWidth)),
    points, s);
}

void draw_rect(Bitmap& bmp, const IntRect& r, const BorderSettings& s){
  DISPATCH(draw_rect_f, s.paint,
    ColorFromColor(s.paint.GetColor()),
    ColorFromPattern(s.paint.GetPattern()),
    ColorFromGradient(s.paint.GetGradient(), r),
    r, s);
}

void erase_but(Bitmap& bmp, const Color& keep, const Paint& paint){
  auto is_not_color = [&bmp, &keep](int x, int y){
    return get_color_raw(bmp, x, y) != keep;
  };
  DISPATCH(set_pixels_if, paint,
    ColorFromColor(paint.GetColor()),
    ColorFromPattern(paint.GetPattern()),
    ColorFromGradient(paint.GetGradient(),
      IntRect(IntPoint(0,0),bmp.GetSize())),
    is_not_color);
}

template<typename Functor>
void fill_ellipse_f(Bitmap& bmp, const Functor& setPixFunc, const IntRect& r){
  IntPoint radiuses(floored(floated(IntPoint(r.w, r.h)) / 2.0));
  int x0 = r.x + radiuses.x;
  int y0 = r.y + radiuses.y;
  int a = radiuses.x;
  int b = radiuses.y;

  if (a == 0){
    vertical_f(bmp, r.Top(), r.Bottom(), r.Left(), setPixFunc);
    return;
  }
  if (b == 0){
    scanline_f(bmp, r.Left(), r.Right(), r.Top(), setPixFunc);
    return;
  }
  int a2 = 2 * a * a;
  int b2 = 2 * b * b;
  int error = a * a * b;

  int x = 0;
  int y = b;

  int stopy = 0;
  int stopx = a2 * b;

  int xoffset = r.w % 2 == 0 ? 1 : 0;
  int yoffset = r.h % 2 == 0 ? 1 : 0;

  while (stopy <= stopx) {
    scanline_f(bmp, x0 - x, x0 + x - xoffset, y0 + y - yoffset, setPixFunc);
    if (y != 0){
      scanline_f(bmp, x0 - x, x0 + x - xoffset, y0 - y, setPixFunc);
    }
    x++;

    error -= b2 * (x - 1);
    stopy += b2;
    if (error <= 0) {
      error += a2 * (y - 1);
      y--;
      stopx -= a2;
    }
  }

  error = b*b*a;
  x = a;
  y = 0;
  stopy = b2*a;
  stopx = 0;
  while (stopy >= stopx) {
    scanline_f(bmp, x0 - x, x0 + x - xoffset, y0 + y - yoffset, setPixFunc);
    if (y != 0){
      scanline_f(bmp, x0 - x, x0 + x - xoffset, y0 - y, setPixFunc);
    }
    y++;
    error -= a2 * (y - 1);
    stopx += a2;
    if (error < 0) {
      error += b2 * (x - 1);
      x--;
      stopy -= b2;
    }
  }
}

void fill_ellipse(Bitmap& bmp, const IntRect& r, const Paint& paint){
  DISPATCH(fill_ellipse_f, paint,
    ColorFromColor(paint.GetColor()),
    ColorFromPattern(paint.GetPattern()),
    ColorFromGradient(paint.GetGradient(), r),
    r);
}

void fill_ellipse_color(Bitmap& bmp, const IntRect& r, const Color& c){
  fill_ellipse_f(bmp, ColorFromColor(c), r);
}

void fill_polygon(Bitmap& bmp, const std::vector<IntPoint>& points,
  const Paint& paint)
{
  DISPATCH(fill_polygon_f, paint,
    ColorFromColor(paint.GetColor()),
    ColorFromPattern(paint.GetPattern()),
    ColorFromGradient(paint.GetGradient(), bounding_rect(points)),
    points);
}

void fill_rect(Bitmap& bmp, const IntRect& r, const Paint& paint){
  DISPATCH(fill_rect_f, paint,
    ColorFromColor(paint.GetColor()),
    ColorFromPattern(paint.GetPattern()),
    ColorFromGradient(paint.GetGradient(), r),
    r);
}

void fill_rect_color(Bitmap& bmp, const IntRect& r, const Color& c){
  fill_rect_f(bmp, ColorFromColor(c), r);
}

void fill_rect_rgb(Bitmap& bmp, const IntRect& r, const ColRGB& c){
  fill_rect_f(bmp, ColorFromColor(Color(c,255)), r);
}

void fill_triangle(Bitmap& bmp, const IntPoint& p0,
  const IntPoint& p1,
  const IntPoint& p2,
  const Paint& paint)
{
  DISPATCH(fill_triangle_f, paint,
    ColorFromColor(paint.GetColor()),
    ColorFromPattern(paint.GetPattern()),
    ColorFromGradient(paint.GetGradient(), bounding_rect(p0,p1,p2)), // Fixme: When used for arrowheads, this will use a different gradient offset than that of the line
    p0, p1, p2);
}

void fill_triangle_color(Bitmap& bmp,
  const IntPoint& p0,
  const IntPoint& p1,
  const IntPoint& p2,
  const Color& c){
  fill_triangle_f(bmp, ColorFromColor(c), p0, p1, p2);
}

Bitmap flip(const Bitmap& bmp, const along& axis){
  return axis.Get() == Axis::HORIZONTAL ?
    flip_horizontal(bmp) :
    flip_vertical(bmp);
}

Bitmap flip(const Bitmap& bmp, const across& axis){
  return axis.Get() == Axis::HORIZONTAL ?
    flip_vertical(bmp) :
    flip_horizontal(bmp);
}

void flood_fill_color(Bitmap& bmp, const IntPoint& pos,
  const Color& fillColor)
{
  std::vector< IntPoint > v;
  const Color targetColor = get_color(bmp, pos);
  if (targetColor == fillColor){
    return;
  }

  v.push_back(pos);
  for (uint i = 0; i != v.size(); i++){
    if (get_color(bmp, v[i]) == targetColor){
      IntPoint w = v[i];
      IntPoint e = v[i];
      for (EVER){
        put_pixel(bmp, w, fillColor);
        if (w.x - 1 < 0 || get_color_raw(bmp, w.x - 1, w.y) != targetColor){
          break;
        }
        w.x -= 1;
        if (w.y != 0 && get_color_raw(bmp, w.x, w.y - 1) == targetColor){
          v.push_back(IntPoint(w.x, w.y - 1));
        }
        if (w.y != bmp.m_h - 1 && get_color_raw(bmp, w.x, w.y + 1) ==
          targetColor)
        {
          v.push_back(IntPoint(w.x, w.y + 1));
        }
      }

      for (EVER){
        put_pixel(bmp, e, fillColor); // Fixme: Will put twice due to w
        if (e.y != 0 && get_color_raw(bmp, e.x, e.y - 1) == targetColor){
          v.push_back(IntPoint(e.x, e.y - 1));
        }
        if (e.y != bmp.m_h - 1 && get_color_raw(bmp, e.x, e.y + 1) ==
          targetColor)
        {
          v.push_back(IntPoint(e.x, e.y + 1));
        }

        if (e.x + 1 == bmp.m_w || get_color_raw(bmp, e.x + 1, e.y) !=
          targetColor)
        {
          break;
        }
        e.x += 1;
      }
    }
  }
}

void flood_fill_gradient(Bitmap& bmp, const IntPoint& pos,
  const Gradient& gradient)
{
  std::vector<IntPoint> v;
  const Color targetColor = get_color(bmp, pos);
  Color fillColor = targetColor == color_black ? color_white : color_black;

  Bitmap filled(bmp.GetSize(), fillColor == color_white ? color_black :
    color_white);
  v.push_back(pos);

  // Min and max filled pixels, used to determine the gradient rectangle
  int min_x = bmp.m_w - 1;
  int min_y = bmp.m_h - 1;
  int max_x = 0;
  int max_y = 0;

  for (uint i = 0; i != v.size(); i++){
    if (get_color(bmp, v[i]) == targetColor){
      IntPoint w = v[i];
      IntPoint e = v[i];
      for (EVER){
        put_pixel(filled, w, fillColor);
        put_pixel(bmp, w, fillColor);
        min_x = std::min(min_x, w.x);
        max_x = std::max(max_x, w.x);
        min_y = std::min(min_y, w.y);
        max_y = std::max(max_y, w.y);

        if (w.x - 1 < 0 || get_color_raw(bmp, w.x - 1, w.y) != targetColor){
          break;
        }
        w.x -= 1;
        if (w.y != 0 && get_color_raw(bmp, w.x, w.y - 1) == targetColor){
          v.push_back(IntPoint(w.x, w.y - 1));
        }
        if (w.y != bmp.m_h - 1 && get_color_raw(bmp, w.x, w.y + 1) ==
          targetColor)
        {
          v.push_back(IntPoint(w.x, w.y + 1));
        }
      }

      for (EVER){
        put_pixel(filled, e, fillColor); // Fixme: Will put twice due to w
        put_pixel(bmp, e, fillColor);
        min_x = std::min(min_x, e.x);
        max_x = std::max(max_x, e.x);
        min_y = std::min(min_y, e.y);
        max_y = std::max(max_y, e.y);
        if (e.y != 0 && get_color_raw(bmp, e.x, e.y - 1) == targetColor){
          v.push_back(IntPoint(e.x, e.y - 1));
        }
        if (e.y != bmp.m_h - 1 && get_color_raw(bmp, e.x, e.y + 1) ==
          targetColor)
        {
          v.push_back(IntPoint(e.x, e.y + 1));
        }

        if (e.x + 1 == bmp.m_w || get_color_raw(bmp, e.x + 1, e.y) !=
          targetColor)
        {
          break;
        }
        e.x += 1;
      }
    }
  }
  if (min_x > max_x || min_y > max_y){
    return;
  }

  Bitmap grBmp(IntSize(max_x - min_x + 1, max_y - min_y + 1), Paint(gradient));
  set_alpha(grBmp, 255);
  for (int y = 0; y != grBmp.m_h; y++){
    for (int x = 0; x != grBmp.m_w; x++){
      if (get_color_raw(filled, x + min_x, y + min_y) != fillColor){
        put_pixel_raw(grBmp, x, y, color_transparent_white);
      }
    }
  }
  blend(offsat(grBmp, min_x, min_y), onto(bmp));
}

void flood_fill_pattern_relative(Bitmap& bmp, const IntPoint& pos, const
  Pattern& pattern)
{
  // Fixme: Use ColorFromPattern
  Bitmap filled(bmp);
  std::vector< IntPoint > v;
  const Color targetColor = get_color(bmp, pos);
  Color c = color_black;
  if (c == targetColor){
    c = color_white;
  }

  const Bitmap& patBmp(pattern.GetBitmap());
  const IntPoint patOffset = pattern.GetAnchor();

  v.push_back(pos);
  for (uint i = 0; i != v.size(); i++){
    if (get_color(bmp, v[i]) == targetColor){
      IntPoint w = v[i];
      IntPoint e = v[i];
      for (EVER){
        put_pixel(bmp, w, c);
        put_pixel(filled, w, get_color_modulo(patBmp, w + patOffset));
        if (w.x - 1 < 0 || get_color_raw(bmp, w.x - 1, w.y) != targetColor){
          break;
        }
        w.x -= 1;
        if (w.y != 0 && get_color_raw(bmp, w.x, w.y - 1) == targetColor){
          v.push_back(IntPoint(w.x, w.y - 1));
        }
        if (w.y != bmp.m_h - 1 && get_color_raw(bmp, w.x, w.y + 1) ==
          targetColor)
        {
          v.push_back(IntPoint(w.x, w.y + 1));
        }
      }

      for (EVER){
        put_pixel(bmp, e, c);
        put_pixel(filled, e, get_color_modulo(patBmp, (e + patOffset)));
        if (e.y != 0 && get_color_raw(bmp, e.x, e.y - 1) == targetColor){
          v.push_back(IntPoint(e.x, e.y - 1));
        }
        if (e.y != bmp.m_h - 1 && get_color_raw(bmp, e.x, e.y + 1) ==
          targetColor)
        {
          v.push_back(IntPoint(e.x, e.y + 1));
        }

        if (e.x + 1 == bmp.m_w || get_color_raw(bmp, e.x + 1, e.y) !=
          targetColor)
        {
          break;
        }
        e.x += 1;
      }
    }
  }
  blit(at_top_left(filled), onto(bmp));
}

void flood_fill(Bitmap& bmp, const IntPoint& pos, const Paint& paint){
  visit(paint,
    [&](const Color& color){
      flood_fill_color(bmp, pos, color);
    },
    [&](const Pattern& pattern){
      flood_fill_pattern_relative(bmp, pos, pattern);
    },
    [&](const Gradient& gradient){
      flood_fill_gradient(bmp, pos, gradient);
    });
}

std::vector<Color> get_palette(const Bitmap& bmp){
  // Using a set of Color with custom hasher to unsigned int
  // was a lot slower than just storing unsigned int, at least in
  // VS2013.
  std::unordered_set<unsigned int> hashed;
  for (int y = 0; y != bmp.m_h; y++){
    for (int x = 0; x != bmp.m_w; x++){
      hashed.insert(to_hash(get_color_raw(bmp, x, y)));
    }
  }

  std::vector<Color> colors;
  colors.reserve(hashed.size());
  for (auto value : hashed){
    colors.push_back(color_from_hash(value));
  }
  return colors;
}

bool overextends_down(const IntRect& r, const Bitmap& bmp){
  return r.h - r.y > bmp.m_h;
}

bool overextends_left(const IntRect& r, const Bitmap&){
  return r.x < 0;
}

bool overextends_right(const IntRect& r, const Bitmap& bmp){
  return r.w - r.x > bmp.m_w;
}

bool overextends_up(const IntRect& r, const Bitmap&){
  return r.y < 0;
}

void rect(Bitmap& bmp, const IntRect& inRect,
  const Optional<BorderSettings>& fg, const Optional<Paint>& bg)
{
  IntRect r(fg.VisitSimple(
    [&](const BorderSettings& s){
      return s.Dashed() ? deflated(inRect, s.lineWidth/2) : inRect;
    },
    inRect));

  bg.Visit([&](const Paint& f){
      fill_rect(bmp, r, f);
  });

  fg.Visit([&](const BorderSettings& b){
    draw_rect(bmp, r, b);
  });
}

void replace_color_color(Bitmap& bmp, const OldColor& in_oldColor,
  const NewColor& in_newColor)
{
  const Color& oldColor(in_oldColor.Get());
  const Color& newColor(in_newColor.Get());
  for (int y = 0; y != bmp.m_h; y++){
    uchar* row = bmp.m_data + y * bmp.m_row_stride;
    for (int x = 0; x != bmp.m_w * BPP; x += BPP){
      color_ptr current(row + x);
      if (current == oldColor){
        current.Set(newColor);
      }
    }
  }
}

void replace_color_pattern(Bitmap& bmp, const OldColor& in_oldColor,
  const Pattern& pattern)
{
  const Color& oldColor(in_oldColor.Get());
  const Bitmap& patBmp(pattern.GetBitmap());
  const IntPoint patOffset(pattern.GetAnchor());
  const IntSize sz(bmp.GetSize());
  for (int y = 0; y != sz.h; y++){
    for (int x = 0; x != sz.w; x++){
      if (get_color_raw(bmp, x,y) == oldColor){
        put_pixel_raw(bmp, x, y, get_color_modulo(patBmp, IntPoint(x, y) +
          patOffset));
      }
    }
  }
}

void replace_color_gradient(Bitmap& bmp, const OldColor& in_oldColor,
  const Gradient& gradient)
{
  const Color& oldColor(in_oldColor.Get());
  auto r = find_color_extents(bmp, in_oldColor.Get());
  r.Visit(
    [&](const IntRect& r){
      set_pixels_if(bmp, ColorFromGradient(gradient, r),
        [&](int x, int y){
          return get_color_raw(bmp, x, y) == oldColor;
        });
    });
}

void replace_color(Bitmap& bmp, const OldColor& oldColor, const Paint& paint){
  visit(paint,
    [&](const Color& color){
      replace_color_color(bmp, oldColor, New(color));
    },
    [&](const Pattern& pattern){
      replace_color_pattern(bmp, oldColor, pattern);
    },
    [&](const Gradient& gradient){
      replace_color_gradient(bmp, oldColor, gradient);
    });
}

// Note: rotate is implemented in rotate.cpp

Bitmap rotate_90cw(const Bitmap& src){
  Bitmap dst(transposed(src.GetSize()));
  uchar* pDst = dst.m_data;
  uchar* pSrc = src.m_data;
  for (int y = 0; y != src.m_h; y++){
    for (int x = 0 ; x != src.m_w; x++){
      int iSrc = y * src.m_row_stride + x * BPP;
      int iDst = x * dst.m_row_stride + (dst.m_w - y - 1) * BPP;
      pDst[ iDst + iR ] = pSrc[iSrc + iR ];
      pDst[ iDst + iG ] = pSrc[iSrc + iG ];
      pDst[ iDst + iB ] = pSrc[iSrc + iB ];
      pDst[ iDst + iA ] = pSrc[iSrc + iA ];
    }
  }
  return dst;
}

Bitmap scale(const Bitmap& bmp, const Scale& scale, ScaleQuality quality){
  switch (quality){
  case ScaleQuality::NEAREST:
    return scale_nearest(bmp, scale);
  case ScaleQuality::BILINEAR:
    return scale_bilinear(bmp, scale);
  };
  assert(false);
  return Bitmap();
}

Bitmap scale_bilinear(const Bitmap& src, const Scale& scale){
  IntSize newSize(constrained(truncated(floated(src.GetSize()) * abs(scale)),
      min_t(1), min_t(1)));
  if (newSize == src.GetSize()){
    return Bitmap(src);
  }

  Bitmap dst(newSize);
  const coord x_ratio = floated(src.m_w - 1) / floated(newSize.w);
  const coord y_ratio = floated(src.m_h - 1) / floated(newSize.h);
  const uchar* data = src.m_data;

  for (int yDst = 0; yDst != newSize.h; yDst++){
    for (int xDst = 0; xDst != newSize.w; xDst++){
      int xSrc = truncated(x_ratio * xDst);
      int ySrc = truncated(y_ratio * yDst);
      const coord x_diff = x_ratio * xDst - xSrc;
      const coord y_diff = y_ratio * yDst - ySrc;
      const int srcIndex = ySrc * src.m_row_stride + xSrc * BPP;
      const_color_ptr a(data + srcIndex);
      const_color_ptr b(data + srcIndex + BPP);
      const_color_ptr c(data + srcIndex + src.m_row_stride);
      const_color_ptr d(data + srcIndex + src.m_row_stride + BPP);

      // Fixme: Check for possible overflow
      const uchar blue = static_cast<uchar>((a.b)*(1-x_diff)*(1-y_diff) +
        (b.b)*(x_diff)*(1-y_diff) +
        (c.b)*(y_diff)*(1-x_diff) +
        (d.b)*(x_diff*y_diff) + 0.5);

      const uchar green = static_cast<uchar>(a.g*(1-x_diff)*(1-y_diff) +
        b.g*(x_diff)*(1-y_diff) +
        (c.g)*(y_diff)*(1-x_diff) +
        (d.g)*(x_diff*y_diff) + 0.5);

      const uchar red = static_cast<uchar>(a.r * (1-x_diff)*(1-y_diff) +
        b.r*(x_diff)*(1-y_diff) +
        c.r*(y_diff)*(1-x_diff) +
        (d.r)*(x_diff*y_diff) + 0.5);

      const uchar alpha = static_cast<uchar>(a.a * (1-x_diff)*(1-y_diff) +
        b.a*(x_diff)*(1-y_diff) +
        c.a*(y_diff)*(1-x_diff) +
        (d.a)*(x_diff*y_diff) + 0.5);

      uchar* rDst = dst.m_data + yDst * (dst.m_row_stride) + xDst * BPP;
      *(rDst + iR) = red;
      *(rDst + iG) = green;
      *(rDst + iB) = blue;
      *(rDst + iA) = alpha;
    }
  }

  if (scale.x < 0){
    dst = flip_horizontal(dst);
  }
  if (scale.y < 0) {
    dst = flip_vertical(dst);
  }
  return dst;
}

Bitmap scale_nearest(const Bitmap& src, int scale){
  const int w2 = src.m_w * scale;
  const int h2 = src.m_h * scale;

  Bitmap scaled(IntSize(w2, h2));
  int x_ratio = (src.m_w << 16) / scaled.m_w + 1;
  int y_ratio = (src.m_h << 16) / scaled.m_h + 1;
  int x2, y2 ;
  uchar* p_dst = scaled.m_data;
  const uchar* p_src = src.m_data;

  for (int i = 0; i< h2; i++){ // Fixme: y
    for (int j = 0; j < w2; j++){ // Fixme: y
      x2 = ((j*x_ratio)>>16);
      y2 = ((i*y_ratio)>>16);

      uchar* rDst = p_dst + i * (scaled.m_row_stride) + j * BPP;
      const uchar* rSrc = p_src + y2 * (src.m_row_stride) + x2 * BPP;

      *(rDst + 0) = *rSrc;
      *(rDst + 1) = *(rSrc + 1);
      *(rDst + 2) = *(rSrc + 2);
      *(rDst + 3) = *(rSrc + 3);
    }
  }
  return scaled;
}

Bitmap scale_nearest(const Bitmap& src, const Scale& scale){
  const int w2 = static_cast<int>(src.m_w * scale.x);
  const int h2 = static_cast<int>(src.m_h * scale.y);

  Bitmap scaled(IntSize(w2, h2));
  int x_ratio = (src.m_w << 16) / scaled.m_w + 1;
  int y_ratio = (src.m_h << 16) / scaled.m_h + 1;
  int x2, y2 ;
  uchar* p_dst = scaled.m_data;
  const uchar* p_src = src.m_data;

  for (int i = 0; i< h2; i++){
    for (int j = 0; j < w2; j++){
      x2 = ((j*x_ratio)>>16);
      y2 = ((i*y_ratio)>>16);

      uchar* rDst = p_dst + i * (scaled.m_row_stride) + j * BPP;
      const uchar* rSrc = p_src + y2 * (src.m_row_stride) + x2 * BPP;

      *(rDst + 0) = *rSrc;
      *(rDst + 1) = *(rSrc + 1);
      *(rDst + 2) = *(rSrc + 2);
      *(rDst + 3) = *(rSrc + 3);
    }
  }
  return scaled;
}

Bitmap scaled_subbitmap(const Bitmap& src, const Scale& scale,
  const IntRect& r)
{
  assert(r.x >= 0);
  assert(r.y >= 0);
  IntSize newSize(rounded(r.w * scale.x), rounded(r.h * scale.y));
  newSize.w = std::max(newSize.w, 1);
  newSize.h = std::max(newSize.h, 1);
  Bitmap scaled(newSize);
  coord x_ratio = 1.0 / scale.x;
  coord y_ratio = 1.0 / scale.y;

  const uchar* data = src.m_data;
  for (int i = 0; i != newSize.h; i++){
    for (int j = 0; j != newSize.w; j++){
      int x = truncated(x_ratio * j + r.x);
      int y = truncated(y_ratio * i + r.y);
      coord x_diff = (x_ratio * j + r.x) - x;
      coord y_diff = (y_ratio * i + r.y) - y;
      int index = y * src.m_row_stride + x * BPP;

      const_color_ptr a(data + index);
      const_color_ptr b(data + index + BPP);
      const_color_ptr c(data + index + src.m_row_stride);
      const_color_ptr d(data + index + src.m_row_stride + BPP);

      uchar blue = static_cast<uchar>((a.b)*(1-x_diff)*(1-y_diff) +
        (b.b)*(x_diff)*(1-y_diff) +
        (c.b)*(y_diff)*(1-x_diff) +
        (d.b)*(x_diff*y_diff));

      uchar green = static_cast<uchar>(a.g*(1-x_diff)*(1-y_diff) +
        b.g*(x_diff)*(1-y_diff) +
        (c.g)*(y_diff)*(1-x_diff) +
        (d.g)*(x_diff*y_diff));

      uchar red = static_cast<uchar>(a.r * (1-x_diff)*(1-y_diff) +
        b.r*(x_diff)*(1-y_diff) +
        c.r*(y_diff)*(1-x_diff) +
        (d.r)*(x_diff*y_diff));

      uchar alpha = static_cast<uchar>(a.a * (1-x_diff)*(1-y_diff) +
        b.a*(x_diff)*(1-y_diff) +
        c.a*(y_diff)*(1-x_diff) +
        (d.a)*(x_diff*y_diff));

      uchar* rDst = scaled.m_data + i * (scaled.m_row_stride) + j * BPP;

      *(rDst + iR) = red;
      *(rDst + iG) = green;
      *(rDst + iB) = blue;
      *(rDst + iA) = alpha;
    }
  }
  return scaled;
}

void set_alpha(Bitmap& bmp, uchar a){
  uchar* data = bmp.GetRaw();
  const int stride = bmp.GetStride();
  IntSize sz(bmp.GetSize());
  for (int y = 0; y != sz.h; y++){
    for (int x = 0; x != sz.w; x++){
      int i = y * stride + x * BPP;
      data[i + iA] = a;
    }
  }
}

Bitmap subbitmap(const Bitmap& orig, const IntRect& r){
  int origStride = orig.m_row_stride;
  const uchar* origData = orig.m_data;

  Bitmap bmp(r.GetSize());
  uchar* data = bmp.GetRaw();
  int destStride = bmp.m_row_stride;

  for (int y = 0; y != r.h; y++){
    for (int x = 0; x != r.w; x++){
      int dstPos = y * destStride + x * BPP;
      int srcPos = (y + r.y) * origStride + (x + r.x) * BPP;
      data[ dstPos ] = origData[ srcPos ];
      data[ dstPos + 1 ] = origData[ srcPos + 1 ];
      data[ dstPos + 2 ] = origData[ srcPos + 2 ];
      data[ dstPos + 3 ] = origData[ srcPos + 3 ];
    }
  }
  return bmp;
}

void vertical_scanline(Bitmap& bmp, int x, const Color& c){
  draw_line(bmp, {{x, 0}, {x, bmp.m_h}}, solid_1px(c));
}

void horizontal_scanline(Bitmap& bmp, int y, const Color& c){
  draw_line(bmp, {{0, y}, {bmp.m_w, y}}, solid_1px(c));
}

} // namespace faint
