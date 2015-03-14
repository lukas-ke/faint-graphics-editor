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

#ifndef FAINT_POINT_HH
#define FAINT_POINT_HH
#include "geo/primitive.hh"

namespace faint{

class Point{
public:
  constexpr Point() : x(0), y(0){}
  constexpr Point(coord x, coord y) : x(x), y(y){}
  static constexpr Point Both(coord v){ return Point(v, v); }
  bool operator==(const Point&) const;
  bool operator!=(const Point&) const;
  Point operator-() const;
  void operator*=(coord);
  void operator+=(const Point&);

  coord x;
  coord y;
};

Point operator-(const Point&, const Point&);
Point operator+(const Point&, const Point&);
Point operator*(const Point&, const Point&);
Point operator*(const Point&, coord scale);
Point operator*(coord scale, const Point&);
Point operator*(const Point&, coord);
Point operator/(const Point&, coord);
Point operator/(const Point&, const Point&);
Point operator%(const Point&, coord);
Point abs(const Point&);
Point max_coords(const Point&, const Point&);
Point max_coords(const Point&, const Point&, const Point&);
Point max_coords(const Point&, const Point&, const Point&, const Point&);
Point min_coords(const Point&, const Point&);
Point min_coords(const Point&, const Point&, const Point&);
Point min_coords(const Point&, const Point&, const Point&, const Point&);
Point transposed(const Point&);

// Verifies that neither coordinate is NAN.
bool valid(const Point&);

} // namespace

#endif
