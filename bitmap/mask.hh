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

#ifndef FAINT_MASK_HH
#define FAINT_MASK_HH
#include "geo/int-size.hh"
#include "geo/primitive.hh"

namespace faint{

class Mask{
public:
  Mask(const IntSize&);
  ~Mask();
  void Set(int x, int y, bool);
  bool Get(int x, int y) const;
  IntSize GetSize() const;
  bool Any() const;
private:
  IntSize m_size;
  bool* m_data;
};

class Bitmap;
class Color;

// A mask which is set for all pixels matching Color.
Mask mask_set_color(const Bitmap&, const Color&);

// A mask which is set in all pixels not matching Color.
Mask mask_not_color(const Bitmap&, const Color&);

// A mask which is set for all pixels with the given alpha
Mask mask_alpha_equal(const Bitmap&, uchar alpha);

} // namespace

#endif
