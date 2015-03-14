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

#ifndef FAINT_ADJUST_HH
#define FAINT_ADJUST_HH
#include "geo/geo-fwd.hh"

namespace faint{

// Return an adjusted version of p2 ("p3") such that the angle between the lines
// (origin, p3) and y=0 is the closest to the angle between (origin, p2) and y=0
// in the set [angle * n + offset].
Point adjust_to(const Point& origin,
  const Point&,
  const Angle& step,
  const Angle& offset);

// Returns the point adjusted to 45-degrees relative to origin
Point adjust_to_45(const Point& origin, const Point&);

// Adjusts like AdjustTo, but allows adding another snap-to angle (altAngle)
Point adjust_to_default(const Point& origin,
  const Point&,
  const Angle&,
  const Angle& altAngle);

enum class ConstrainDir{NONE, HORIZONTAL, VERTICAL};

// Constrains point to lie on a horizontal or vertical straight line
// intersecting origin.
ConstrainDir constrain_pos(Point&, const Point& origin);
ConstrainDir constrain_pos(IntPoint&, const IntPoint& origin);
void constrain_pos(IntPoint&, const IntPoint& origin, ConstrainDir);
void constrain_pos(Point&, const Point& origin, ConstrainDir);
Point constrain_proportional(const Point& moved,
  const Point& opposite,
  const Point& origin);

} // namespace

#endif
