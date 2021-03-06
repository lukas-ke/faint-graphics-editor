// -*- coding: us-ascii-unix -*-
// Copyright 2015 Lukas Kemmer
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

#ifndef FAINT_SCALE_NEAREST_HH
#define FAINT_SCALE_NEAREST_HH
#include "bitmap/bitmap.hh"

namespace faint{

// Returns a scaled copy of the bitmap (using nearest neighbour
// interpolation).
Bitmap scale_nearest(const Bitmap&, const IntSize&);

// Returns a uniformly scaled copy of the bitmap (using nearest
// neighbour interpolation).
Bitmap scale_nearest(const Bitmap&, int scale);

} // namespace

#endif
