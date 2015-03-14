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

#ifndef FAINT_TOOL_UTIL_HH
#define FAINT_TOOL_UTIL_HH
#include "util/pos-info.hh"
#include "util/template-fwd.hh"

namespace faint{

class ObjText;

// Whether actions should use the foreground or background color
// depending on modifiers etc.
ColorSetting fg_or_bg(const PosInfo&);

class category_toolutil;

// Flag controlling if the fill of an object which is not currently filled
// should be considered for hit testing
using include_hidden_fill = Distinct<bool, category_toolutil, 0>;

// Flag controlling whether pixels in the floating selection should be
// considered (=true), or the pixels below (=false)
using include_floating_selection = Distinct<bool, category_toolutil, 1>;

// Returns the color at the given position.
// Note: The position must must be inside.
Color get_color(const Either<Bitmap, ColorSpan>&, const IntPoint&);

// Returns the Paint under the given position. Includes objects if in
// the object layer.
Paint get_hovered_paint(const PosInside&,
  const include_hidden_fill&,
  const include_floating_selection&);

enum class SearchMode{exact_object, include_grouped};
// Retrieves the hovered selected object as a text object, or 0 if no
// object is hovered, the object isn't selected or is not a text
// object.
ObjText* hovered_selected_text(const PosInfo&,
  SearchMode searchMode=SearchMode::exact_object);

Paint offsat_if_object_aligned(const Paint&, const IntPoint& offset);

bool outside_canvas(const PosInfo&);
bool outside_canvas_by(const PosInfo&, int pixels);

// True if the edge or inside of an object was hit, and the object
// supports ts_Fg or ts_Bg
bool object_color_region_hit(const PosInfo&);

// Returns p constrained relative to anchor so that a square is formed.
Point constrain_to_square(const Point& anchor, const Point& p, bool subPixel);

} // namespace

#endif
