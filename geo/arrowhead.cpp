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

#include "geo/angle.hh"
#include "geo/arrowhead.hh"
#include "geo/line.hh"
#include "geo/measure.hh"
#include "geo/rect.hh"

namespace faint{

Arrowhead::Arrowhead(const Point& p0, const Point& p1, const Point& p2)
  : m_p0(p0),
    m_p1(p1),
    m_p2(p2)
{}

Rect Arrowhead::BoundingBox() const{
  Point p0(min_coords(m_p0, m_p1, m_p2));
  Point p1(max_coords(m_p0, m_p1, m_p2));
  return Rect(p0, p1);
}

Point Arrowhead::LineAnchor() const{
  return mid_point(m_p0, m_p2);
}

Point Arrowhead::P0() const{
  return m_p0;
}

Point Arrowhead::P1() const{
  return m_p1;
}

Point Arrowhead::P2() const{
  return m_p2;
}

Arrowhead get_arrowhead(const LineSegment& l, coord lineWidth){
  Angle angle = line_angle(reversed(l));
  Angle orth = angle + pi / 2.0;
  coord ax0 = cos(orth) * 9 * (lineWidth / 3.0);
  coord ay0 = sin(orth) * 9 * (lineWidth / 3.0);

  coord ax1 = cos(angle) * 15 * (lineWidth / 2.0);
  coord ay1 = sin(angle) * 15 * (lineWidth / 2.0);
  coord x1(l.p1.x);
  coord y1(l.p1.y);
  Point p1(x1 + ax0 + ax1, y1 + ay0 + ay1);
  Point p2(x1, y1);
  Point p3(x1 - ax0 + ax1, y1 - ay0 + ay1);
  return Arrowhead(p1, p2, p3);
}

} // namespace
