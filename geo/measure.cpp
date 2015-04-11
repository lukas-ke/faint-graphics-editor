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
#include "geo/line.hh"
#include "geo/measure.hh"
#include "geo/pathpt.hh"
#include "geo/point.hh"
#include "geo/rect.hh"
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

IntRect bounding_rect(const IntPoint& p0, const IntPoint& p1){
  return IntRect(p0, p1);
}

IntRect bounding_rect(const IntPoint& p0, const IntPoint& p1, const IntPoint& p2){
  return IntRect(min_coords(p0, p1, p2), max_coords(p0, p1, p2));
}

Rect bounding_rect(const Rect& r1, const Rect& r2){
  return union_of(r1, r2);
}

Rect bounding_rect(const LineSegment& line){
  return bounding_rect(line.p0, line.p1);
}

IntRect bounding_rect(const IntLineSegment& line){
  return IntRect(line.p0, line.p1);
}

Rect bounding_rect(const Point& p0, const Point& p1){
  return Rect(p0, p1);
}

Rect bounding_rect(const Point& p0, const Point& p1, const Point& p2){
  return Rect(min_coords(p0, p1, p2), max_coords(p0,p1, p2));
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
  if (pts.empty()){
    return {};
  }

  std::vector<Point> midPts;
  for (size_t i = 1; i != pts.size(); i++){
    midPts.push_back(mid_point(pts[i-1], pts[i]));
  }
  return midPts;
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
