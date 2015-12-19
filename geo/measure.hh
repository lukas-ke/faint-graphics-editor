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

#ifndef FAINT_MEASURE_HH
#define FAINT_MEASURE_HH
#include <vector>
#include "geo/geo-fwd.hh"
#include "geo/primitive.hh"

namespace faint{

// Returns the angle of the line in the interval [0, 2pi],
// with angles increasing counter-clockwise.
// <../doc/angle360_ccw.png>
Angle angle360_ccw(const LineSegment&);

// Note: returns an empty rectangle situated at IntPoint,
// This is required by the variadic bounding_rect implementation.
IntRect bounding_rect(const IntPoint&);

IntRect bounding_rect(const IntPoint&, const IntPoint&);
IntRect bounding_rect(const IntLineSegment&);
IntRect bounding_rect(const IntRect&, const IntRect&);

// Note: Returns an empty rectangle situated at Point.
// This is required by the variadic bounding_rect implementation.
Rect bounding_rect(const Point&);

Rect bounding_rect(const Point&, const Point&);
Rect bounding_rect(const Rect&);
Rect bounding_rect(const Rect&, const Rect&);
Rect bounding_rect(const LineSegment&);


template<class A, class ...B>
auto bounding_rect(const A& head, const B&... tail)
  -> decltype(bounding_rect(head))
{
  return bounding_rect(bounding_rect(head), bounding_rect(tail...));
}

// Returns the positive distance between the points. Insensitive to
// ordering.
coord distance(const IntPoint&, const IntPoint&);
coord distance(const Point&, const Point&);

// Approximates the length of the Bezier curve from start, using the
// line distance for the specified number of subdivisions. A higher
// number of subdivisions gives a better approximation, but is more
// time consuming.
coord distance(const Point& start, const CubicBezier&, int subdivisions);

// a and b are diameters
coord ellipse_perimeter(coord a, coord b);
coord ellipse_perimeter(const Radii&);

// Returns the angle between the positive x-axis and the line in the
// interval (-pi, pi], with positive angles on the lower semi-circle
// (Pretty much the same as atan2!)
// <../doc/line_angle_cw.png>
Angle line_angle_cw(const LineSegment&);

Point mid_point(const LineSegment&);
Point mid_point(const Point&, const Point&);

// Returns the mid_point between all points in the vector. The
// returned vector will contain one point less than source vector,
// or no points if the source was empty.
std::vector<Point> mid_points(const std::vector<Point>&);

// Returns the points with the mid-points between adjacent points
// interspersed.
// For {p1, p2, p3}, returns {p1, m(p1, p2), p2, m(p2, p3), p3}
std::vector<Point> with_mid_points(const std::vector<Point>&);

// Returns the points with the mid-points between adjacent points
// interspersed, including the mid-point between the last and first
// points (as implied by cyclic).
// For {p1, p2, p3}, returns {m(p3, p1), p1, m(p2, p1), p2, m(p2, p3), p3}
std::vector<Point> with_mid_points_cyclic(const std::vector<Point>&);

coord perimeter(const std::vector<PathPt>&);

} // namespace

#endif
