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

#include <algorithm> // transform
#include <iterator> // back_inserter
#include "geo/angle.hh"
#include "geo/geo-func.hh"
#include "geo/int-rect.hh"
#include "geo/int-size.hh"
#include "geo/line.hh"
#include "geo/padding.hh"
#include "geo/pathpt.hh"
#include "geo/rect.hh"
#include "geo/scale.hh"
#include "geo/size.hh"

namespace faint{

IntPoint delta_x(int dx){
  return IntPoint(dx, 0);
}

Point delta_x(coord dx){
  return Point(dx, 0);
}

IntPoint delta_xy(int dx, int dy){
  return IntPoint(dx, dy);
}

Point delta_xy(coord dx, coord dy){
  return Point(dx, dy);
}

IntPoint delta_y(int dy){
  return IntPoint(0, dy);
}

Point delta_y(coord dy){
  return Point(0, dy);
}

Point floated(const IntPoint& p){
  return Point(static_cast<coord>(p.x), static_cast<coord>(p.y));
}

Rect floated(const IntRect& r){
  return Rect(floated(r.TopLeft()), floated(r.GetSize()));
}

Size floated(const IntSize& sz){
  return Size(static_cast<coord>(sz.w), static_cast<coord>(sz.h));
}

std::vector<IntPoint> floored(const std::vector<Point>& points){
  std::vector<IntPoint> v2;
  v2.reserve(points.size());
  std::transform(begin(points), end(points), std::back_inserter(v2),
    [](const Point& pt){return floored(pt);});
  return v2;
}

IntRect padded(const IntRect& r, const Padding& p){
  return IntRect(r.TopLeft() - IntPoint(p.left, p.top),
    r.BottomRight() + IntPoint(p.right, p.bottom));
}

Point point_from_size(const Size& sz){
  return Point(sz.w, sz.h);
}

IntPoint point_from_size(const IntSize& sz){
  return IntPoint(sz.w, sz.h);
}

Radii radii_from_point(const Point& pt){
  return Radii(pt.x, pt.y);
}

IntRect rect_from_size(const IntSize& sz){
  return IntRect(IntPoint(0,0), sz);
}

Rect rect_from_size(const Size& sz){
  return Rect(Point(0,0), sz);
}

static IntPoint ceiled(const Point& p){
  return IntPoint(ceiled(p.x), ceiled(p.y));
}

IntPoint floored(const Point& p){
  return IntPoint(floored(p.x), floored(p.y));
}

IntPoint rounded(const Point& p){
  return IntPoint(rounded(p.x), rounded(p.y));
}

IntPoint rounded_down(const Point& p){
  return IntPoint(rounded_down(p.x), rounded_down(p.y));
}

IntPoint rounded_up(const Point& p){
  return IntPoint(rounded_up(p.x), rounded_up(p.y));
}

IntRect floiled(const Rect& r){
  return IntRect(floored(r.TopLeft()), ceiled(r.BottomRight()));
}

IntLineSegment floored(const LineSegment& segment){
  return IntLineSegment(floored(to_pair(segment)));
}

IntLineSegment rounded(const LineSegment& segment){
  return IntLineSegment(rounded(segment.p0), rounded(segment.p1));
}

PathPt rotate_point(const PathPt& pt, const Angle& angle, const Point& origin){
  return pt.Rotated(angle, origin);
}

Point rotate_point(const Point& pt, const Angle& angle, const Point& origin){
  Point p2 = pt - origin;
  return Point(p2.x * cos(angle) - p2.y * sin(angle),
    p2.x * sin(angle) + p2.y * cos(angle)) + origin;
}

Point scale_point(const Point& pt, const Scale& scale, const Point& origin){
  Point p2 = pt - origin;
  p2.x *= scale.x;
  p2.y *= scale.y;
  return p2 + origin;
}

IntSize rounded(const Size& sz){
  return IntSize(rounded(sz.w), rounded(sz.h));
}

PathPt scale_point(const PathPt& pt, const Scale& sc, const Point& origin){
  PathPt p2(pt);
  p2.p = scale_point(p2.p, sc, origin);
  p2.c = scale_point(p2.c, sc, origin);
  p2.d = scale_point(p2.d, sc, origin);
  p2.r.x = (pt.p.x + pt.r.x - origin.x) * sc.x + origin.x - p2.p.x;
  p2.r.y = (pt.p.y + pt.r.y - origin.y) * sc.x + origin.y - p2.p.y;
  return p2;
}

IntSize size_from_point(const IntPoint& pt){
  return IntSize(pt.x, pt.y);
}

IntSize truncated(const Size& sz){
  return IntSize(truncated(sz.w), truncated(sz.h));
}

IntPoint truncated(const Point& pt){
  return IntPoint(truncated(pt.x), truncated(pt.y));
}

IntRect floored(const Rect& r){
  return IntRect(floored(r.TopLeft()), truncated(r.GetSize()));
}

IntSize floored(const Size& sz){
  return IntSize(floored(sz.w), floored(sz.h));
}

Point polar(const Radii& r, const Angle& a){
  return Point(r.x * cos(a), r.y * sin(a));
}

IntLineSegment left_side(const IntRect& r){
  return {r.TopLeft(), r.BottomLeft()};
}

IntLineSegment top_side(const IntRect& r){
  return {r.TopLeft(), r.TopRight()};
}

LineSegment left_side(const Rect& r){
  return {r.TopLeft(), r.BottomLeft()};
}

LineSegment right_side(const Rect& r){
  return {r.TopRight(), r.BottomRight()};
}

LineSegment top_side(const Rect& r){
  return {r.TopLeft(), r.TopRight()};
}

LineSegment bottom_side(const Rect& r){
  return {r.BottomLeft(), r.BottomRight()};
}

std::pair<int, int> left_right(const IntRect& r){
  return {r.Left(), r.Right()};
}

std::pair<int, int> top_bottom(const IntRect& r){
  return {r.Top(), r.Bottom()};
}

IntRect centered(const IntSize& item, const IntSize& span){
  return IntRect(IntPoint(span.w / 2 - item.w / 2,
      span.h / 2- item.h / 2),
    span);
}

} // namespace
