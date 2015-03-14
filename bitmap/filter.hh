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

#ifndef FAINT_FILTER_HH
#define FAINT_FILTER_HH
#include <vector>
#include "bitmap/bitmap-fwd.hh"
#include "bitmap/paint-fwd.hh"
#include "geo/geo-fwd.hh"
#include "geo/range.hh"

namespace faint{

class Filter;

class brightness_contrast_t{
  // Parameters for apply_brightness_and_contrast. This is applied
  // like a gain and bias, with brightness as bias and contrast as
  // gain.
public:
  constexpr brightness_contrast_t(double brightness, double contrast)
    : brightness(brightness), contrast(contrast)
  {}

  double brightness = 0.0;
  double contrast = 1.0;
};

// Returns a copy with adjusted brightness and contrast.
// Alpha values are copied unmodified from the source image.
Bitmap brightness_and_contrast(const Bitmap&, const brightness_contrast_t&);

void desaturate_simple(Bitmap&);
void desaturate_weighted(Bitmap&);

using pixelize_range_t = LowerBoundedInt<1>;
void pixelize(Bitmap&, const pixelize_range_t&);

void sepia(Bitmap&, int intensity);
Bitmap subtract(const Bitmap& lhs, const Bitmap& rhs);
Bitmap unsharp_mask_fast(const Bitmap&, double blurSigma);
Bitmap unsharp_mask_exact(const Bitmap&, double blurSigma);

Filter* get_shadow_filter();
Filter* get_invert_filter();
Filter* get_pinch_whirl_filter();
Filter* get_pixelize_filter();
Filter* get_stroke_filter();
void invert(Bitmap&);

using color_range_t = StaticBoundedInterval<0,255>;

void color_balance(Bitmap&,
  const color_range_t& r,
  const color_range_t& g,
  const color_range_t& b);

void filter_pinch_whirl(Bitmap& bmp, coord pinch, const Angle& whirl);

using threshold_range_t = StaticBoundedInterval<0,765>;
void threshold(Bitmap&, const threshold_range_t&,
  const Paint& inside, const Paint& outside);

std::vector<int> threshold_histogram(const Bitmap&);

std::vector<int> red_histogram(Bitmap&);
std::vector<int> green_histogram(Bitmap&);
std::vector<int> blue_histogram(Bitmap&);

} // namespace

#endif
