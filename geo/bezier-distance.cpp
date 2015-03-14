// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
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
#include "geo/measure.hh"
#include "geo/pathpt.hh"

namespace faint{

coord distance(const Point& from, const CubicBezier& to, int subdivisions){
  assert(subdivisions > 0);

  Point previous(from);
  coord length = 0.0;
  for (int i = 1; i <= subdivisions; i++){
    const double t(static_cast<double>(i) / subdivisions);
    const Point current =
      from * std::pow(1.0 - t, 3) +
      3.0 * to.c * std::pow(1.0 - t, 2) * t +
      3.0 * to.d * (1.0 - t) * std::pow(t, 2) +
      to.p * std::pow(t, 3);

    length += distance(previous, current);
    previous = current;
  }

  return length;
}

} // namespace
