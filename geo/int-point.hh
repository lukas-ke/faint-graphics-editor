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

#ifndef FAINT_INT_POINT_HH
#define FAINT_INT_POINT_HH
#include "geo/primitive.hh"

namespace faint{

class IntPoint{
public:
  constexpr IntPoint() : x(0), y(0){}
  constexpr IntPoint(int x, int y) : x(x), y(y){}

  static constexpr IntPoint Both(int v){ return IntPoint(v, v); }
  void operator+=(const IntPoint&);
  void operator-=(const IntPoint&);
  bool operator==(const IntPoint&) const;
  bool operator!=(const IntPoint&) const;
  IntPoint operator-() const;
  bool operator<(const IntPoint&) const;
  IntPoint(coord, coord) = delete;
  IntPoint(float, float) = delete;
  IntPoint(size_t, size_t) = delete;

  int x;
  int y;
};

bool fully_positive(const IntPoint&);
IntPoint max_coords(const IntPoint&, const IntPoint&);
IntPoint max_coords(const IntPoint&, const IntPoint&, const IntPoint&);
IntPoint min_coords(const IntPoint&, const IntPoint&);
IntPoint min_coords(const IntPoint&, const IntPoint&, const IntPoint&);
IntPoint operator-(const IntPoint&, const IntPoint&);
IntPoint operator-(const IntPoint&, int);
IntPoint operator+(const IntPoint&, int);
IntPoint operator+(const IntPoint&, const IntPoint&);
IntPoint operator*(const IntPoint&, const IntPoint&);
IntPoint operator/(const IntPoint&, const IntPoint&);
IntPoint operator/(const IntPoint&, int);
IntPoint operator*(const IntPoint&, int);
IntPoint operator*(int, const IntPoint&);

class Point;
Point operator*(const IntPoint&, coord);
Point operator*(coord scale, const IntPoint&);
Point operator/(const IntPoint&, coord);

} // namespace

#endif
