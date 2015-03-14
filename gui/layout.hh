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

#ifndef FAINT_LAYOUT_HH
#define FAINT_LAYOUT_HH
#include "bitmap/color.hh"
#include "geo/int-size.hh"

// Fixme: Rename to e.g. ui-constants or smth
namespace faint{

const int panel_padding = 10;
const int item_spacing = 10;
constexpr IntSize big_button_size(85, 100);
const int labelSpacing = 10;

namespace ui{

constexpr IntSize horizontal_slider_size(200, 20);
constexpr IntSize tall_horizontal_slider_size(200, 50);
constexpr ColRGB nice_red(181, 0, 0);
constexpr ColRGB nice_green(34, 177, 76);
constexpr ColRGB nice_blue(47, 54, 153);

}} // namespace

#endif
