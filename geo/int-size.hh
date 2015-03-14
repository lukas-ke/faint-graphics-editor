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

#ifndef FAINT_INT_SIZE_HH
#define FAINT_INT_SIZE_HH
#include "geo/primitive.hh"

namespace faint{

class IntSize{
public:
  constexpr IntSize() : w(0), h(0){}
  constexpr IntSize(int w, int h) : w(w), h(h){}

  static constexpr IntSize Both(int v){ return IntSize(v, v); }

  bool operator==(const IntSize&) const;
  bool operator!=(const IntSize&) const;

  using value_type = int;
  value_type w;
  value_type h;

  // Prevent construction from float. Use truncated(Size(w,h)) instead.
  IntSize(coord, coord) = delete;
  IntSize(float, float) = delete;
  IntSize(size_t, size_t) = delete;
};

int area(const IntSize&);
bool area_less(const IntSize& lhs, int rhs);
IntSize min_coords(const IntSize&, const IntSize&);
IntSize max_coords(const IntSize&, const IntSize&);
IntSize operator+(const IntSize&, const IntSize&);
IntSize operator-(const IntSize&, const IntSize&);
IntSize operator*(const IntSize&, int);
IntSize operator*(int, const IntSize&);
IntSize operator*(const IntSize&, const IntSize&);
IntSize operator/(const IntSize&, int);
IntSize transposed(const IntSize&);

class Size;
Size operator*(const IntSize&, coord);
Size operator*(coord, const IntSize&);
Size operator/(const IntSize&, coord);

} // namespace

#endif
