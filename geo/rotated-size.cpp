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

#include <algorithm> // min, max
#include <cmath>
#include "geo/rotated-size.hh"

namespace faint{

IntSize get_rotated_size(const Angle& angle, const IntSize& size){
  // Use the upper-left corner as pivot
  const coord CtX = 0;
  const coord CtY = 0;

  // Find the corners to initialize the destination width and height
  const coord cA = cos(angle);
  const coord sA = sin(angle);

  const coord x1 = CtX + (-CtX) * cA - (-CtY) * sA;
  const coord x2 = CtX + (size.w - CtX) * cA - (-CtY) * sA;
  const coord x3 = CtX + (size.w - CtX) * cA - (size.h - CtY) * sA;
  const coord x4 = CtX + (-CtX) * cA - (size.h - CtY) * sA;

  const coord y1 = CtY + (-CtY) * cA + (-CtX) * sA;
  const coord y2 = CtY + (size.h - CtY) * cA + (-CtX) * sA;
  const coord y3 = CtY + (size.h - CtY) * cA + (size.w - CtX) * sA;
  const coord y4 = CtY + (-CtY) * cA + (size.w - CtX) * sA;

  const IntPoint offset(floored(std::min({x1, x2, x3, x4})),
    floored(std::min({y1, y2, y3, y4})));
  const IntSize newSize(ceiled(std::max({x1, x2, x3, x4})) - offset.x,
    ceiled(std::max({y1,y2,y3,y4})) - offset.y);

  return newSize;
}

} // namespace
