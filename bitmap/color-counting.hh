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

#ifndef FAINT_COLOR_COUNTING_HH
#define FAINT_COLOR_COUNTING_HH
#include <map>
#include <unordered_map>
#include <vector>
#include "bitmap/color.hh"

namespace faint{

class Bitmap;
class Mask;

// Maps Colors to a pixel count. The colors are indexed by their hash.
// Hashing externally, without a hasher-template argument, was for
// some reason faster, at least with Visual C++
using color_counts_t = std::unordered_map<color_hash, int>;

// Adds the colors from the Bitmap to the passed in color_counts_t
void add_color_counts(const Bitmap&, color_counts_t&);

int count_colors(const Bitmap&);

// Returns a vector containing all distinct colors (after stripping
// alpha) in the bitmap sorted by operator<. Masked pixels are
// excluded.
std::vector<ColRGB> unique_colors_rgb(const Bitmap&, const Mask& exclude);

// Returns a copy with all fully transparent color variants
// (i.e. any rgba = {*,*,*,0} removed in favor of a single instance
// of the given preferredRgb with alpha=0, at the end.
//
// If there are no transparent pixels, the resulting vector will be
// identical to the source.
std::vector<Color> merged_fully_transparent(const std::vector<Color>&,
  const ColRGB& preferredRgb);

// Returns the most common color. The color_counts_t must not be empty
Color most_common(const color_counts_t&);

} // namespace

#endif
