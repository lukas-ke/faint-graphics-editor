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

#ifndef FAINT_ARROWHEAD_HH
#define FAINT_ARROWHEAD_HH
#include "geo/point.hh"

namespace faint{

class LineSegment;
class Rect;

class Arrowhead{
  // An arrowhead specified by three points.
public:
  Rect BoundingBox() const;
  // The point where a line should attach to the arrowhead
  Point LineAnchor() const;
  Point P0() const;
  Point P1() const;
  Point P2() const;
private:
  friend Arrowhead get_arrowhead(const LineSegment&, coord);
  Arrowhead(const Point&, const Point&, const Point&);
  Point m_p0;
  Point m_p1;
  Point m_p2;
};

// Returns an arrowhead with the arrow-tip at the end of the
// LineSegment.
// If the line and arrowhead are drawn, the line should be drawn up
// to the Arrowhead::LineAnchor() instead of LineSegment::p1.
Arrowhead get_arrowhead(const LineSegment&, coord lineWidth);

} // namespace

#endif
