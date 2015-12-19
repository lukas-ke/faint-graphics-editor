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
#include "geo/bezier.hh"
#include "geo/measure.hh"

namespace faint{

using std::pow;

static coord B0(coord t){
  return pow(1.0 - t, 3);
}

static coord B1(coord t){
  return 3 * t * pow(1.0 - t, 2);
}

static coord B2(coord t){
  return 3 * pow(t, 2) * (1.0 - t);
}

static coord B3(coord t){
  return pow(t, 3);
}

Point bezier_point(coord t, const Point& from, const CubicBezier& to){
  return
    B0(t) * from +
    B1(t) * to.c +
    B2(t) * to.d +
    B3(t) * to.p;
}

coord distance(const Point& from, const CubicBezier& to, int subdivisions){
  assert(subdivisions > 0);

  Point previous(from);
  coord length = 0.0;
  const double div = static_cast<double>(subdivisions);
  for (int i = 1; i <= subdivisions; i++){
    const double t = i / div;
    const Point current = bezier_point(t, from, to);
    length += distance(previous, current);
    previous = current;
  }

  return length;
}

Beziers in_twain(const Point& from, const CubicBezier& to){
  // Pilfered from http://jeremykun.com/tag/de-casteljau/

  std::vector<Point> v = {from, to.c, to.d, to.p};
  auto mp0 = mid_points(v);
  auto mp1 = mid_points(mp0);
  auto mp2 = mid_points(mp1);
  return {{mp2[0], mp0[0], mp1[0]}, {to.p, mp1[1], mp0[2]}};
}

} // namespace
