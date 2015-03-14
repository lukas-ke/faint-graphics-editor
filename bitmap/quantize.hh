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

#ifndef FAINT_QUANTIZE_HH
#define FAINT_QUANTIZE_HH
#include "bitmap/color-list.hh"

namespace faint{

enum class Dithering{ON, OFF};

enum class OctTreeDepth{
  FOUR=4,
  FIVE=5,
  SIX=6
};

using MappedColors = std::pair<AlphaMap, ColorList>;

// Returns a list of quantized colors and an AlphaMap, corresponding
// to the Bitmap, which indexes into the list.
MappedColors quantized(const Bitmap&,
  Dithering,
  OctTreeDepth d=OctTreeDepth::FIVE);

Bitmap quantized_bmp(const Bitmap&,
  Dithering,
  OctTreeDepth d=OctTreeDepth::FIVE);

// Quantizes the bitmap in place, reducing its color count to at most
// 256. Loses alpha transparency.
void quantize(Bitmap&, Dithering,
  OctTreeDepth d=OctTreeDepth::FIVE);

} // namespace

#endif
