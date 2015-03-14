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
#include <limits>
#include "geo/int-size.hh"
#include "geo/size.hh"

namespace faint{

bool IntSize::operator==(const IntSize& other) const{
  return w == other.w && h == other.h;
}

bool IntSize::operator!=(const IntSize& other) const{
  return !((*this)==other);
}

int area(const IntSize& sz){
  return sz.w * sz.h;
}

IntSize min_coords(const IntSize& lhs, const IntSize& rhs){
  return IntSize(std::min(lhs.w, rhs.w), std::min(lhs.h, rhs.h));
}

IntSize max_coords(const IntSize& lhs, const IntSize& rhs){
  return IntSize(std::max(lhs.w, rhs.w), std::max(lhs.h, rhs.h));
}

IntSize operator+(const IntSize& lhs, const IntSize& rhs){
  return IntSize(lhs.w + rhs.w, lhs.h + rhs.h);
}

IntSize operator-(const IntSize& lhs, const IntSize& rhs){
  return IntSize(lhs.w - rhs.w, lhs.h - rhs.h);
}

IntSize operator*(const IntSize& lhs, const IntSize& rhs){
  return IntSize(lhs.w * rhs.w, lhs.h * rhs.h);
}

IntSize operator*(const IntSize& size, int scalar){
  return IntSize(scalar * size.w, scalar * size.h);
}

IntSize operator*(int scalar, const IntSize& size){
  return size * scalar;
}

IntSize operator/(const IntSize& size, int scalar){
  return IntSize(size.w / scalar, size.h / scalar);
}

IntSize transposed(const IntSize& sz){
  return IntSize(sz.h, sz.w);
}

Size operator*(const IntSize& size, coord scale){
  return Size(size.w * scale, size.h * scale);
}

Size operator*(coord scale, const IntSize& size){
  return size * scale;
}

Size operator/(const IntSize& size, coord scale){
  return Size(size.w / scale, size.h / scale);
}

bool area_less(const IntSize& size, int area){
  assert(size.w >= 0 && size.h >= 0 && area >= 0);
  if (size.w == 0 || size.h == 0){
    return false;
  }
  if (size.w > std::numeric_limits<int>::max() / size.h){
    return false;
  }

  return size.w * size.h < area;
}

} // namespace
