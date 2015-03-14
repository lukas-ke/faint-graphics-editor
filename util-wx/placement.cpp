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

#include "wx/window.h"
#include "geo/int-point.hh"
#include "geo/int-size.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/placement.hh"

namespace faint{

const int spacing = 5; // Fixme: Local spacing, I probably have some other defined

IntPoint below(window_t window){
  return {left_side(window),
    bottom(window) + spacing};
}

int bottom(window_t window){
  return top(window) + get_height(window);
}

int left_side(window_t window){
  return get_pos(window).x;
}

int right_side(window_t window){
  return get_pos(window).x + get_width(window);
}

int top(window_t window){
  return get_pos(window).y;
}

IntPoint to_the_right_of(window_t window){
  return {right_side(window) + spacing, top(window)};
}

IntPoint to_the_left_middle_of(window_t window, const IntSize& size){
  const auto x = get_pos(window).x - spacing - size.w;
  const auto y = top(window) +
    get_height(window) / 2 - size.h / 2;
  return {x, y};
}

IntPoint to_the_right_middle_of(window_t window, const IntSize& size){
  const auto x = left_side(window) + get_width(window) + 5;
  const auto y = top(window) + get_height(window) / 2 - size.h / 2;
  return {x, y};
}

} // namespace
