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

#ifndef FAINT_GEO_FUNC_HH
#define FAINT_GEO_FUNC_HH
#include <utility>
#include <vector>
#include "geo/geo-fwd.hh"
#include "geo/primitive.hh"

namespace faint{

// Single-value point construction
IntPoint delta_x(int);
Point delta_x(coord);
IntPoint delta_xy(int x, int y);
Point delta_xy(coord, coord);
IntPoint delta_y(int);
Point delta_y(coord);
Point floated(const IntPoint&);
Rect floated(const IntRect&);
Size floated(const IntSize&);
Point mid_point(const Point&, const Point&);
std::vector<Point> mid_points(const std::vector<Point>&);
IntRect padded(const IntRect&, const Padding&);
Point point_from_size(const Size&);
IntPoint point_from_size(const IntSize&);
Radii radii_from_point(const Point&);
IntRect rect_from_size(const IntSize&);
Rect rect_from_size(const Size&);
PathPt rotate_point(const PathPt&, const Angle&, const Point& origin);
Point rotate_point(const Point&, const Angle&, const Point& origin);
Point scale_point(const Point&, const Scale&, const Point& origin);
IntPoint rounded(const Point&);
IntPoint rounded_down(const Point&);
IntPoint rounded_up(const Point&);
IntSize rounded(const Size&);

// Floors the left, top, ceils the right and bottom, so that the
// IntRect includes all pixels the Rect intersects.
// <../doc/floiled.png>
IntRect floiled(const Rect&);

PathPt scale_point(const PathPt&, const Scale&, const Point& origin);
IntSize size_from_point(const IntPoint&);
IntSize truncated(const Size&);

IntPoint truncated(const Point&);
IntPoint floored(const Point&);
IntRect floored(const Rect&);
IntSize floored(const Size&);
IntLineSegment floored(const LineSegment&);
IntLineSegment rounded(const LineSegment&);
Point polar(const Radii&, const Angle&);

template<typename T1, typename T2>
auto floored(const std::pair<T1, T2>& p){
  return std::make_pair(floored(p.first), floored(p.second));
}

std::vector<IntPoint> floored(const std::vector<Point>&);

IntLineSegment left_side(const IntRect&);
IntLineSegment top_side(const IntRect&);

LineSegment left_side(const Rect&);
LineSegment right_side(const Rect&);
LineSegment top_side(const Rect&);
LineSegment bottom_side(const Rect&);

std::pair<int, int> left_right(const IntRect&);
std::pair<int, int> top_bottom(const IntRect&);

// Returns the bounding rectangle for something with the size of "item"
// centered in a region the size of "span".
IntRect centered(const IntSize& item, const IntSize& span);

} // namespace

#endif
