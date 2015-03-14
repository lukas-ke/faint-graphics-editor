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

#ifndef FAINT_AUTO_CROP_HH
#define FAINT_AUTO_CROP_HH
#include "util/template-fwd.hh"
#include "geo/geo-fwd.hh"

namespace faint{
class Bitmap;
class Color;

// Returns an IntRect bounding all pixels of the specified color, or
// nothing if the color is not found.
Optional<IntRect> find_color_extents(const Bitmap&, const Color&);

// Returns either zero, one or two regions that can be cropped to.
// The regions are determined by removing contiguous colors at the
// edges. Two regions are possible if one side of the image is all one
// color and the opposite edge is of a different color.
AtMost2<IntRect> get_auto_crop_rectangles(const Bitmap&);

// Overload for optional bitmap. Returns nothing if the bitmap is not
// set.
AtMost2<IntRect> get_auto_crop_rectangles(const Optional<Bitmap>&);

// Determines the color under the IntRect edge in the Bitmap.
// Returns an un-set Optional if the edge intersects more than one
// color.
Optional<Color> get_edge_color(const Bitmap&, const IntRect&);

// Returns the most common color along the Bitmap edges in the directions
// the newRect would grow its canvas.
// Fixme: Move somewhere more suitable
Optional<Color> resize_dwim_color(const Bitmap&, const IntRect& newRect);

// Returns the most common color along the Bitmap edge
Color most_common_edge_color(const Bitmap&);

bool get_bottom_edge_color(const Bitmap&, Color& result);
bool get_left_edge_color(const Bitmap&, Color& result);
bool get_right_edge_color(const Bitmap&, Color& result);
bool get_top_edge_color(const Bitmap&, Color& result);

}
#endif
