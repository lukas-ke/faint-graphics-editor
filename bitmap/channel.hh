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

#ifndef FAINT_CHANNEL_HH
#define FAINT_CHANNEL_HH
#include <vector>

namespace faint{

class Bitmap;
class Color;

using channel_t = std::vector<unsigned char>;

class Channels{
  // Red, green, blue and alpha color channels.
  // Some algorithms operate better on color channels than
  // a combined array (as in faint::Bitmap).
public:
  channel_t r;
  channel_t g;
  channel_t b;
  channel_t a;

  // The row width
  int w;
};

Channels separate_into_channels(const Bitmap&);
Bitmap combine_into_bitmap(const Channels&);
Color get_color(const Channels&, int index);
std::vector<Color> get_colors(const Channels&);

} // namespace

#endif
