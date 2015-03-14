// -*- coding: us-ascii-unix -*-
// Copyright 2013 Lukas Kemmer
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
#include "geo/radii.hh"

namespace faint{

bool Radii::operator==(const Radii& other) const{
  return coord_eq(x, other.x) && coord_eq(y, other.y);
}

bool Radii::operator!=(const Radii& other) const{
  return !operator==(other);
}

void Radii::operator*=(coord sc){
  x *= sc;
  y *= sc;
}

Radii abs(const Radii& r){
  return Radii(std::fabs(r.x), std::fabs(r.y));
}

Radii operator*(const Radii& lhs, const Radii& rhs){
  return Radii(lhs.x * rhs.x, lhs.y * rhs.y);
}

} // namespace
