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

#ifndef FAINT_GEO_LIST_POINTS_HH
#define FAINT_GEO_LIST_POINTS_HH
#include <array>
#include <vector>
#include "geo/geo-fwd.hh"

namespace faint{

std::vector<IntPoint> as_polygon(const IntRect&);
std::array<Point,4> corners(const Rect&);
std::array<Point,4> points_number_order(const Tri&);
std::array<Point,4> points_clockwise(const Tri&);

IntRect bounding_rect(const std::vector<IntPoint>&);
Rect bounding_rect(const std::vector<Point>&);

// Returns a vector of cubic bezier spline control points and end
// points approximating the ellipse described by the Tri
std::vector<PathPt> ellipse_as_path(const Tri&);

} // namespace

#endif
