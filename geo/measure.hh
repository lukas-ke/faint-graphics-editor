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

// Same as IntRect(const IntPoint&, const IntPoint&), but
// sometimes looks more consistent
IntRect bounding_rect(const IntPoint&, const IntPoint&);
IntRect bounding_rect(const IntPoint&, const IntPoint&, const IntPoint&);
IntRect bounding_rect(const IntLineSegment&);
Rect bounding_rect(const Rect&, const Rect&);
Rect bounding_rect(const LineSegment&);
Rect bounding_rect(const Point&, const Point&, const Point&);
Rect bounding_rect(const Point&, const Point&);

coord distance(const IntPoint&, const IntPoint&);
coord distance(const Point&, const Point&);

// Approximates the length of the Bezier curve from start, using the
// line distance for the specified number of subdivisions. A higher
// number of subdivisions gives a better approximation, but is more
// time consuming.
coord distance(const Point& start, const CubicBezier&, int subdivisions);

coord ellipse_perimeter(coord a, coord b);

// Returns the angle between the positive x-axis and the line in the
// interval [-pi, pi], with positive angles on the lower semi-circle
// <../doc/line_angle_cw.png>
Angle line_angle_cw(const LineSegment&);

Point mid_point(const LineSegment&);
Point mid_point(const Point&, const Point&);
std::vector<Point> mid_points(const std::vector<Point>&);

coord perimeter(const std::vector<PathPt>&);

} // namespace

#endif
