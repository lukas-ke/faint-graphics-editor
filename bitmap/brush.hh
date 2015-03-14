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

#ifndef FAINT_BRUSH_HH
#define FAINT_BRUSH_HH
#include "geo/geo-fwd.hh"
#include "geo/primitive.hh"

namespace faint{

class Brush{
public:
  explicit Brush(const IntSize&);
  Brush(const Brush&);
  ~Brush();
  void Set(int x, int y, uchar value);
  void Set(const IntPoint&, uchar value);
  uchar Get(int x, int y) const;
  IntSize GetSize() const;
  Brush& operator=(const Brush&);
private:
  uchar* m_data;
  int m_w;
  int m_h;
};

Brush circle_brush(int w);
Brush rect_brush(int w);
Brush experimental_brush(int w);

} // namespace

#endif
