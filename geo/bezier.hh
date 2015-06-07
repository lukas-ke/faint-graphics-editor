// -*- coding: us-ascii-unix -*-
// Copyright 2015 Lukas Kemmer
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

#ifndef FAINT_BEZIER_HH
#define FAINT_BEZIER_HH
#include "geo/pathpt.hh"

namespace faint{

class Beziers{
public:
  Beziers(const CubicBezier& first, const CubicBezier& second)
    : first(first), second(second)
  {}
  CubicBezier first;
  CubicBezier second;
};

// Returns the point on the bezier at the given offset
// (0 <= t <= 1.0)
Point bezier_point(coord t, const Point&, const CubicBezier&);

Beziers in_twain(const Point& from, const CubicBezier& to);

} // namespace

#endif
