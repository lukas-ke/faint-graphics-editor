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

#ifndef FAINT_PLACEMENT_HH
#define FAINT_PLACEMENT_HH
#include "util-wx/window-types-wx.hh"

namespace faint{

class IntSize;
class IntPoint;

// Functions for relative window placement.
//
// Used in un-resizable dialogs and such with layout requirements
// which are tricky to do with sizers. In general, sizers should
// be preferred.
//
// <../doc/placement.png>
//
// - The functions that return an IntPoint also add some spacing in
//   the main direction, e.g:
//   below(w) -> {left_side(w), bottom(w) + SPACING}
//
// - The functions that return the coordinate of a side (bottom, left_side,
//   right_side, top) return the coordinate without spacing.

int bottom(window_t);
int left_side(window_t);
int right_side(window_t);
int top(window_t);

IntPoint below(window_t);
IntPoint to_the_right_of(window_t);
IntPoint to_the_left_middle_of(window_t, const IntSize&);
IntPoint to_the_right_middle_of(window_t, const IntSize&);

} // namespace

#endif
