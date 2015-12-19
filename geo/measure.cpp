// -*- coding: us-ascii-unix -*-
// Copyright 2013 Lukas Kemmer
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
#include <cmath>
#include "geo/angle.hh"
#include "geo/geo-func.hh"
#include "geo/int-rect.hh"
#include "geo/int-size.hh"
#include "geo/line.hh"
#include "geo/measure.hh"
#include "geo/pathpt.hh"
#include "geo/point.hh"
#include "geo/rect.hh"
#include "geo/size.hh"
#include "util/iter.hh"

namespace faint{

Angle angle360_ccw(const LineSegment& l){
  if (l.p0 == l.p1){
    return Angle::Zero();
  }

  Angle angle = line_angle_cw(l);
  if (angle < Angle::Zero()){
    angle = angle + tau;
  }
  if (angle == Angle::Zero()){
    return angle;
  }
  return tau - angle;
}

IntRect bounding_rect(const IntPoint& p){
  return {p, IntSize(0,0)};
}

IntRect bounding_rect(const IntPoint& p0, const IntPoint& p1){
  return {p0, p1};
}

IntRect bounding_rect(const IntRect& r1, const IntRect& r2) {
  const auto tl = min_coords(r1.TopLeft(), r2.TopLeft());
  const auto br = max_coords(r1.BottomRight(), r2.BottomRight());
  return {tl, br};
}

Rect bounding_rect(const Rect& r){
  return r;
}

Rect bounding_rect(const Rect& r1, const Rect& r2){
  const auto tl = min_coords(r1.TopLeft(), r2.TopLeft());
  const auto br = max_coords(r1.BottomRight(), r2.BottomRight());
  return {tl, br};
}

Rect bounding_rect(const LineSegment& line){
  return bounding_rect(line.p0, line.p1);
}

IntRect bounding_rect(const IntLineSegment& line){
  return {line.p0, line.p1};
}

Rect bounding_rect(const Point& p){
  return {p, Size(0,0)};
}

Rect bounding_rect(const Point& p0, const Point& p1){
  return {p0, p1};
}

coord distance(const Point& p0, const Point& p1){
  return sqrt(sq(p0.x - p1.x) + sq(p0.y - p1.y));
}

coord distance(const IntPoint& pt1, const IntPoint& pt2){
  return distance(floated(pt1), floated(pt2));
}

Angle line_angle_cw(const LineSegment& l){
  return atan2(l.p1.y - l.p0.y, l.p1.x - l.p0.x);
}

Point mid_point(const LineSegment& l){
  return mid_point(l.p0, l.p1);
}

Point mid_point(const Point& p0, const Point& p1){
  return (p0 + p1) / 2;
}

std::vector<Point> mid_points(const std::vector<Point>& pts){
  if (pts.size() < 2){
    return {};
  }

  std::vector<Point> midPts;
  midPts.reserve(pts.size() - 1);
  for (size_t i = 1; i != pts.size(); i++){
    midPts.push_back(mid_point(pts[i-1], pts[i]));
  }
  return midPts;
}

std::vector<Point> with_mid_points(const std::vector<Point>& src){
  if (src.size() < 2){
    return src;
  }

  std::vector<Point> dst;
  dst.reserve(src.size() * 2 - 1);

  auto prev = src.front();
  dst.push_back(prev);
  for (const auto& pt : but_first(src)){
    dst.push_back(mid_point(pt, prev));
    dst.push_back(pt);
    prev = pt;
  }
  return dst;
}

std::vector<Point> with_mid_points_cyclic(const std::vector<Point>& src){
  if (src.size() < 2){
    // At least two points are needed to find mid-points, even if
    // cyclic
    return src;
  }

  std::vector<Point> dst;
  dst.reserve(src.size() * 2);

  auto prev = src.back();
  for (const auto& pt : src){
    dst.push_back(mid_point(pt, prev));
    dst.push_back(pt);
    prev = pt;
  }
  return dst;
}

coord ellipse_perimeter(coord a, coord b){
  coord pi = faint::pi.Rad();

  // Ramanujan, Srinivasa, (1914). "Modular Equations and
  // Approximations to pi"
  // <../doc/ellipse_perimeter.png>
  return
   pi * (a+b) * (
     (3 * sq(a - b)) / (
       sq(a + b) * (
         sqrt(
           -3*(sq(a-b))/sq(a+b) + 4
         )
         + 10
       )
     ) + 1
   );
}

coord ellipse_perimeter(const Radii& r){
  return ellipse_perimeter(r.x * 2, r.y * 2);

}
coord perimeter(const std::vector<PathPt>& path){
  coord len = 0.0;

  PathPt start(path.front());
  assert(start.IsMove());
  Point currPos = start.p;

  for (const PathPt& pt : but_first(path)){
    currPos = pt.Visit(
      [&](const ArcTo& arc){
        // Fixme: Straight-line for arc?! Use correct formula.
        len += distance(currPos, arc.p);
        return arc.p;
      },
      [&](const Close&){
        len += distance(start.p, currPos);
        return start.p;
      },
      [&](const CubicBezier& bezier){
        const int bezierIterations = 10;
        len += distance(currPos, bezier, bezierIterations);
        return bezier.p;
      },
      [&](const LineTo& line){
        len += distance(currPos, line.p);
        return line.p;
      },
      [&](const MoveTo& move){
        return move.p;
      });
  }
  return len;
}

} // namespace
