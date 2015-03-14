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

#ifndef FAINT_COLOR_SPAN_HH
#define FAINT_COLOR_SPAN_HH
#include "bitmap/color.hh"
#include "geo/int-size.hh"

namespace faint{

class ColorSpan{
public:
  ColorSpan();
  ColorSpan(const Color&, const IntSize&);
  Color color;
  IntSize size;
};

class IntRect;
bool fully_inside(const IntRect&, const ColorSpan&);
bool invalid_pixel_pos(const IntPoint&, const ColorSpan&);

} // namespace

#endif
