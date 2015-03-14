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

#ifndef FAINT_COLOR_BITMAP_UTIL_HH
#define FAINT_COLOR_BITMAP_UTIL_HH
#include "bitmap/bitmap-fwd.hh"
#include "bitmap/paint-fwd.hh"
#include "geo/geo-fwd.hh"
#include "util/setting-id.hh" // For LineStyle

namespace faint{

// Returns a bitmap filled with the specified color, with increasing
// opacity along the y-axis
Bitmap alpha_gradient_bitmap_vertical(const ColRGB&, const IntSize&);

Bitmap alpha_gradient_bitmap_horizontal(const ColRGB&, const IntSize&);

// Returns a bitmap of the specified size filled with the given
// color.  If the color has alpha, the color will be blended towards
// a checkered pattern.
Bitmap color_bitmap(const Color&, const IntSize&);
Bitmap color_bitmap(const Color&, const IntSize&, const IntSize& patternSize);
Bitmap paint_bitmap(const Paint&, const IntSize&);
Bitmap paint_bitmap(const Paint&, const IntSize&, const IntSize& patternSize);

// Returns a color which doesn't blend too much against the specified
// background when used as a border.
Color discernible_border_color(const Paint&);

// Returns a pattern for initializing the pattern on the pattern tab
// in the color dialog, when not initialized with a pattern.
Bitmap default_pattern(const IntSize&);

Bitmap gradient_bitmap(const Gradient&, const IntSize&);

// Returns a bitmap of hue and saturation values for mid-lightness
// with hue increasing along the X-axis and saturation decreasing
// along the Y-axis
Bitmap hue_saturation_color_map(const IntSize&);

// Returns a bitmap of the given hue and saturation, with lightness
// increasing from 0.0 to 1.0 along the y-axis.
Bitmap lightness_gradient_bitmap(const HS&, const IntSize&);

// Returns a copy of the bitmap with a black 1-pixel border over the
// edge pixels
Bitmap with_border(const Bitmap&, LineStyle=LineStyle::SOLID);
Bitmap with_border(const Bitmap&, const Color&, LineStyle=LineStyle::SOLID);

Bitmap resize_bitmap(const Paint& bg,
  const IntSize&,
  const Bitmap& illustration,
  const IntPoint&);

} // namespace

#endif
