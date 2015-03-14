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

#include <algorithm>
#include <cassert>
#include "geo/geo-func.hh"
#include "geo/geo-list-points.hh"
#include "geo/int-point.hh"
#include "geo/int-rect.hh"
#include "geo/pathpt.hh"
#include "geo/rect.hh"
#include "geo/tri.hh"

namespace faint{

IntRect bounding_rect(const std::vector<IntPoint>& pts){
  assert(!pts.empty());
  const IntPoint& first = pts[0];
  int min_x = first.x;
  int min_y = first.y;
  int max_x = first.x;
  int max_y = first.y;
  for (size_t i = 1; i != pts.size(); i++){
    const IntPoint& pt = pts[i];
    min_x = std::min(min_x, pt.x);
    min_y = std::min(min_y, pt.y);
    max_x = std::max(max_x, pt.x);
    max_y = std::max(max_y, pt.y);
  }
  return IntRect(IntPoint(min_x, min_y), IntPoint(max_x, max_y));
}

Rect bounding_rect(const std::vector<Point>& pts){
  assert(!pts.empty());
  const Point& first = pts[0];
  coord min_x = first.x;
  coord min_y = first.y;
  coord max_x = first.x;
  coord max_y = first.y;
  for (size_t i = 1; i != pts.size(); i++){
    const Point& pt = pts[i];
    min_x = std::min(min_x, pt.x);
    min_y = std::min(min_y, pt.y);
    max_x = std::max(max_x, pt.x);
    max_y = std::max(max_y, pt.y);
  }
  return Rect(Point(min_x, min_y), Point(max_x, max_y));
}

std::array<Point,4> corners(const Rect& r){
  return {{
    r.TopLeft(),
    r.TopRight(),
    r.BottomLeft(),
    r.BottomRight() }};
}

std::array<Point,4> points_number_order(const Tri& tri){
  return {{
    tri.P0(),
    tri.P1(),
    tri.P2(),
    tri.P3()}};
}

std::array<Point,4> points_clockwise(const Tri& tri){
  return {{
    tri.P0(),
    tri.P1(),
    tri.P3(),
    tri.P2()}};
}

std::vector<IntPoint> as_polygon(const IntRect& r){
  return {r.TopLeft(),
    r.TopRight(),
    r.BottomRight(),
    r.BottomLeft()};
}

std::vector<PathPt> ellipse_as_path(const Tri& tri0){
  const coord skew = tri0.Skew();
  Tri tri(skewed(tri0, -skew));
  const Angle angle = tri.GetAngle();
  tri = rotated(tri, -angle, tri.P0());

  const coord x = tri.P0().x;
  const coord y = tri.P0().y;
  const coord dx = tri.Width();
  const coord dy = tri.Height();

  const coord rx = dx / 2.0;
  const coord ry = dy / 2.0;
  const coord t = 0.551784;

  const coord skew_1 = 0;
  const coord skew_2 = skew * (0.5 - 0.5 * t);
  const coord skew_3 = skew * 0.5;
  const coord skew_4 = skew * (0.5 + 0.5 * t);
  const coord skew_5 = skew;

  const Point origin(x,y);
  return {
    // Start
    PathPt::MoveTo(rotate_point(Point(x + rx + skew_1, y + dy), angle, origin)),

    // Segment 1
   PathPt::CubicBezierTo(rotate_point(Point(x + dx + skew_3, y + ry), angle,
       origin),
     rotate_point(Point(x + rx + rx * t + skew_1, y + dy), angle, origin),
     rotate_point(Point(x + dx + skew_2, y + ry + ry * t), angle, origin)),


    PathPt::CubicBezierTo(rotate_point(Point(x + rx + skew_5, y), angle, origin),
      rotate_point(Point(x + dx + skew_4, y + ry - ry * t), angle, origin),
      rotate_point(Point(x + rx + rx * t + skew_5, y), angle, origin)),


    // Segment 3
    PathPt::CubicBezierTo(rotate_point(Point(x + skew_3, y + ry), angle, origin),
      rotate_point(Point(x + rx - rx * t + skew_5, y), angle, origin), // c
      rotate_point(Point(x + skew_4, y + ry - ry * t), angle, origin)), // d


    // Segment 4
    PathPt::CubicBezierTo(rotate_point(Point(x + rx + skew_1, y + dy), angle,
        origin),
      rotate_point(Point(x + skew_2, y + ry + ry * t), angle, origin),
      rotate_point(Point(x + rx - rx * t, y + dy), angle, origin))};
}

} // namespace
