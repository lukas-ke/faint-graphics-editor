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

#include "geo/geo-func.hh"
#include "geo/int-rect.hh"
#include "util/color-span.hh"

namespace faint{

ColorSpan::ColorSpan()
  : color(color_white),
    size(0,0)
{}

ColorSpan::ColorSpan(const Color& color, const IntSize& size)
  : color(color),
    size(size)
{}

bool fully_inside(const IntRect& r, const ColorSpan& span){
  return intersection(r, rect_from_size(span.size)) == r;
}

bool invalid_pixel_pos(const IntPoint& pos, const ColorSpan& span){
  return !rect_from_size(span.size).Contains(pos);
}

} // namespace
