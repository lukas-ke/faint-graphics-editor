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

#ifndef FAINT_GAUSSIAN_BLUR_HH
#define FAINT_GAUSSIAN_BLUR_HH
#include "bitmap/bitmap.hh"

namespace faint {

// Returns the result of applying gaussian blur to the bitmap with a
// radius of sigma.
// Complexity: O(n * sigma).
Bitmap gaussian_blur_exact(const Bitmap&, double sigma);

// Approximation of gaussian blur with consecutive box blurs.
// Complexity: O(n) for n-pixels (unaffected by sigma).
Bitmap gaussian_blur_fast(const Bitmap&, double sigma);

} // namespace

#endif
