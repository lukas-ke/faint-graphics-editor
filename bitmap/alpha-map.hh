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

#ifndef FAINT_ALPHA_MAP_HH
#define FAINT_ALPHA_MAP_HH
#include "geo/int-size.hh"
#include "util/distinct.hh"
#include "util/optional.hh"

namespace faint{

class Brush;
class IntPoint;
class IntRect;

class AlphaMapRef{
  // View of a sub-region in an AlphaMap.
public:
  uchar Get(int x, int y) const;
  IntSize GetSize() const;

  // Returns the rectangle surrounding >0 positions
  Optional<IntRect> BoundingRect() const;
private:
  friend class AlphaMap;
  AlphaMapRef(const uchar*, const IntSize&, int);
  const uchar* m_data;
  IntSize m_size;
  int m_stride;
};

class AlphaMap{
  // A "one-channel" Bitmap for alpha values and what not.
public:
  explicit AlphaMap(const IntSize&);
  AlphaMap(const AlphaMap&);
  ~AlphaMap();
  void Add(int x, int y, uchar value);
  AlphaMapRef FullReference() const;
  uchar Get(int x, int y) const;
  uchar* GetRaw();
  const uchar* GetRaw() const;
  IntSize GetSize() const;
  void Reset(const IntSize&);
  void Set(int x, int y, uchar value);
  AlphaMapRef SubReference(const IntRect&) const;
  AlphaMap SubCopy(const IntRect&) const;

  AlphaMap& operator=(const AlphaMap&) = delete;
private:
  void Initialize(const IntSize&);
  void Clear();
  uchar* m_data;
  IntSize m_size;
  int m_stride;
};

class category_alpha_map;
using UpperLeft = Distinct<IntPoint, category_alpha_map, 0>;

// Brush stroke between from and to, using the given brush. The
// positions refer to the upper-left pixel of the Brush bounding
// rectangle.
void stroke(AlphaMap&, const UpperLeft& from, const UpperLeft& to,
  const Brush&);

} // namespace

#endif
