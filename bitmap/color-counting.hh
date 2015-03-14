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

namespace faint{

class Bitmap;
class Color;

// Maps Colors to a pixel count
using color_counts_t = std::map<Color, int>;

// Adds the colors from the Bitmap to the passed in color_counts_t
void add_color_counts(const Bitmap&, color_counts_t&);

// Returns the most common color. The color_counts_t must not be
// empty
Color most_common(const color_counts_t&);

int count_colors(const Bitmap&);

} // namespace

#endif
