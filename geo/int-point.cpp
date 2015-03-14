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
#include <cassert>
#include "geo/int-point.hh"
#include "geo/point.hh"

namespace faint{

void IntPoint::operator+=(const IntPoint& other){
  x += other.x;
  y += other.y;
}

void IntPoint::operator-=(const IntPoint& other){
  x -= other.x;
  y -= other.y;
}

bool IntPoint::operator==(const IntPoint& other) const{
  return x == other.x && y == other.y;
}

bool IntPoint::operator!=(const IntPoint& other) const{
  return !operator==(other);
}

IntPoint IntPoint::operator-() const {
  return IntPoint(-x, -y);
}

bool IntPoint::operator<(const IntPoint& other) const{
  return y < other.y || (y == other.y && x < other.x);
}

bool fully_positive(const IntPoint& p){
  return p.x >= 0 && p.y >= 0;
}

IntPoint max_coords(const IntPoint& a, const IntPoint& b){
  return IntPoint(std::max(a.x, b.x), std::max(a.y, b.y));
}


IntPoint max_coords(const IntPoint& a, const IntPoint& b, const IntPoint& c){
  return IntPoint(std::max({a.x, b.x, c.x}),
    std::max({a.y, b.y, c.y}));
}

IntPoint min_coords(const IntPoint& a, const IntPoint& b){
  return IntPoint(std::min(a.x, b.x), std::min(a.y, b.y));
}

IntPoint min_coords(const IntPoint& a, const IntPoint& b, const IntPoint& c){
  return IntPoint(std::min({a.x, b.x, c.x}), std::min({a.y, b.y, c.y}));
}

IntPoint operator-(const IntPoint& lhs, const IntPoint& rhs){
  return IntPoint(lhs.x - rhs.x, lhs.y - rhs.y);
}

IntPoint operator-(const IntPoint& p, int delta){
  return IntPoint(p.x - delta, p.y - delta);
}

IntPoint operator+(const IntPoint& p, int delta){
  return IntPoint(p.x + delta, p.y + delta);
}

IntPoint operator+(const IntPoint& lhs, const IntPoint& rhs){
  return IntPoint(lhs.x + rhs.x, lhs.y + rhs.y);
}

IntPoint operator*(const IntPoint& lhs, int rhs){
  return IntPoint(lhs.x * rhs, lhs.y * rhs);
}

IntPoint operator*(int lhs, const IntPoint& rhs){
  return IntPoint(lhs * rhs.x, lhs * rhs.y);
}

IntPoint operator*(const IntPoint& lhs, const IntPoint& rhs){
  return IntPoint(lhs.x * rhs.x, lhs.y * rhs.y);
}

IntPoint operator/(const IntPoint& lhs, const IntPoint& rhs){
  assert(rhs.x != 0);
  assert(rhs.y != 0);
  return IntPoint(lhs.x / rhs.x, lhs.y / rhs.y);
}

IntPoint operator/(const IntPoint& pt, int sc){
  return IntPoint(pt.x / sc, pt.y / sc);
}

Point operator*(const IntPoint& p, coord scale){
  return Point(p.x * scale, p.y * scale);
}

Point operator*(coord scale, const IntPoint& p){
  return operator*(p, scale);
}

Point operator/(const IntPoint& pt, coord scale){
  return Point(pt.x / scale, pt.y / scale);
}

} // namespace
