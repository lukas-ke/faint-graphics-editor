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

#ifndef FAINT_UI_CONSTANTS_HH
#define FAINT_UI_CONSTANTS_HH
#include "bitmap/color.hh"
#include "geo/int-size.hh"
#include "util/distinct.hh"

namespace faint{

class category_ui_constants;
using OuterSpacing = PrimitiveSubtype<int, category_ui_constants, 0>;
using ItemSpacing = PrimitiveSubtype<int, category_ui_constants, 1>;

} // namespace

namespace faint{ namespace ui{

constexpr OuterSpacing panel_padding(10);
constexpr ItemSpacing item_spacing(10);
constexpr ItemSpacing small_item_spacing(5);
constexpr ItemSpacing label_spacing(10);

constexpr IntSize horizontal_slider_size(200, 20);
constexpr IntSize tall_horizontal_slider_size(200, 50);
constexpr IntSize big_button_size(85, 100);

constexpr ColRGB nice_red(181, 0, 0);
constexpr ColRGB nice_green(34, 177, 76);
constexpr ColRGB nice_blue(47, 54, 153);

}} // namespace

#endif
