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

#include <algorithm>
#include "geo/point.hh"

using std::min;
using std::max;

namespace faint{

bool Point::operator==(const Point& other) const{
  return coord_eq(x, other.x) && coord_eq(y, other.y);
}

bool Point::operator!=(const Point& other) const{
  return !operator==(other);
}

Point Point::operator-() const{
  return {-x, -y};
}

void Point::operator*=(coord sc){
  x *= sc;
  y *= sc;
}

void Point::operator+=(const Point& other){
  x += other.x;
  y += other.y;
}

Point operator-(const Point& lhs, const Point& rhs){
  return {lhs.x - rhs.x, lhs.y - rhs.y};
}

Point operator+(const Point& lhs, const Point& rhs){
  return {lhs.x + rhs.x, lhs.y + rhs.y};
}

Point operator*(const Point& p, coord scale){
  return {p.x * scale, p.y * scale};
}

Point operator*(coord scale, const Point& p){
  return {p.x * scale, p.y * scale};
}

Point operator*(const Point& p0, const Point& p1){
  return {p0.x * p1.x, p0.y * p1.y};
}

Point operator/(const Point& p, coord d){
  return {p.x/d, p.y/d};
}

Point operator%(const Point& p, coord denom){
  return {fmod(p.x, denom), fmod(p.y, denom)};
}

Point operator/(const Point& lhs, const Point& rhs){
  return {lhs.x / rhs.x, lhs.y / rhs.y};
}

Point abs(const Point& p){
  return {std::fabs(p.x), std::fabs(p.y)};
}

Point max_coords(const Point& p0, const Point& p1){
  return {std::max(p0.x, p1.x), std::max(p0.y, p1.y)};
}

Point max_coords(const Point& p0, const Point& p1, const Point& p2){
  return {max(max(p0.x, p1.x), p2.x), max(max(p0.y, p1.y), p2.y)};
}

Point max_coords(const Point& p0,
  const Point& p1,
  const Point& p2,
  const Point& p3)
{
  return {max(max(max(p0.x, p1.x), p2.x), p3.x),
    max(max(max(p0.y, p1.y), p2.y), p3.y)};
}

Point min_coords(const Point& p0, const Point& p1){
  return {min(p0.x, p1.x), min(p0.y, p1.y)};
}

Point min_coords(const Point& p0, const Point& p1, const Point& p2){
  return {min(min(p0.x, p1.x), p2.x), min(min(p0.y, p1.y), p2.y)};
}

Point min_coords(const Point& p0,
  const Point& p1,
  const Point& p2,
  const Point& p3)
{
  return {min(min(min(p0.x, p1.x), p2.x), p3.x),
    min(min(min(p0.y, p1.y), p2.y), p3.y)};
}

Point transposed(const Point& pt){
  return {pt.y, pt.x};
}

bool valid(const Point& p){
  return !(std::isnan(p.x) || std::isnan(p.y));
}

} // namespace
