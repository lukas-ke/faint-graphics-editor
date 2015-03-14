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

#include <cmath>
#include "geo/size.hh"

namespace faint{

bool Size::operator==(const Size& other) const {
  return coord_eq(w, other.w) && coord_eq(h, other.h);
}

bool Size::operator!=(const Size& other) const {
    return !((*this)==other);
}

Size operator+(const Size& lhs, const Size& rhs){
  return Size(lhs.w + rhs.w, lhs.h + rhs.h);
}

Size Size::operator/(const Size& other) const{
  return Size(w / other.w, h / other.h);
}

Size operator-(const Size& lhs, const Size& rhs){
  return Size(lhs.w - rhs.w, lhs.h - rhs.h);
}

Size operator/(const Size& lhs, coord scalar){
  return Size(lhs.w / scalar, lhs.h / scalar);
}

coord area(const Size& size){
  return size.w * size.h;
}

bool area_less(const Size& size, coord rhs){
  const coord a = area(size);
  if (!std::isfinite(a)){
    return false;
  }
  return a < rhs; // Fixme
}

} // namespace
