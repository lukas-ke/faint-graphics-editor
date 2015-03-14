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

#ifndef FAINT_TRI_HH
#define FAINT_TRI_HH
#include "geo/angle.hh"
#include "geo/geo-fwd.hh"
#include "geo/point.hh"

namespace faint{

struct Adj{
  // An adjustment of a Tri
  Adj();
  coord scale_x;
  coord scale_y;
  coord tr_x;
  coord tr_y;
  Angle angle;
  coord skew;
};

class Tri {
  // A skewable bounding box defined by three points, p0-p2, with a
  // fourth projected, p3.
public:
  constexpr Tri() = default;
  Tri(const Point& p0, const Point& p1, coord h);
  Tri(const Point& p0, const Angle&, const Size&);
  Tri(const Point& p0, const Point& p1, const Point& p2);
  Angle GetAngle() const;

  // The width is the distance from P0() to P1()
  coord Width() const;

  // The height is the distance from P0() to P2()
  coord Height() const;
  coord Skew() const;
  Point P0() const;
  Point P1() const;
  Point P2() const;
  Point P3() const;
  bool Contains(const Point& pt) const;
  bool operator==(const Tri&) const;
private:
  friend Adj get_adj(const Tri&, const Tri&);
  friend bool valid(const Tri&);
  Point m_p0;
  Point m_p1;
  Point m_p2;
};

coord area(const Tri&);
Rect bounding_rect(const Tri&);
Point center_point(const Tri&);
Adj get_adj(const Tri&, const Tri&);
Radii get_radii(const Tri&);
Point mid_P0_P1(const Tri&);
Point mid_P0_P2(const Tri&);
Point mid_P1_P3(const Tri&);
Point mid_P2_P3(const Tri&);
LineSegment P0_P1(const Tri&);
LineSegment P0_P2(const Tri&);
Tri offset_aligned(const Tri&, coord alongP0_P1, coord alongP0_P2);
Tri rotated(const Tri&, const Angle& angle, Point origin);
Tri scaled(const Tri&, const Scale&, const Point& origin);
Tri skewed(const Tri& t, coord skewX);
Tri translated(const Tri&, coord tX, coord tY);
Tri tri_from_rect(const Rect&);

// Verifies that no point in the Tri contains invalid values.
bool valid(const Tri&);
bool rather_zero(const Tri&);

} // namespace

#endif
