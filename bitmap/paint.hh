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

#ifndef FAINT_PAINT_HH
#define FAINT_PAINT_HH
#include <cassert>
#include <type_traits> // result_of
#include "bitmap/paint-fwd.hh"

namespace faint{

class IntPoint;

class Paint{
  // A color, pattern or gradient.
public:
  Paint();
  explicit Paint(const Color&);
  explicit Paint(const Gradient&);
  explicit Paint(const Pattern&);
  Paint(const Paint&);
  Paint(Paint&&);
  ~Paint();
  bool IsColor() const;
  bool IsGradient() const;
  bool IsPattern() const;
  const Color& GetColor() const;
  const Gradient& GetGradient() const;
  const Pattern& GetPattern() const;
  Paint& operator=(const Paint&);
private:
  class PaintImpl;
  PaintImpl* m_impl;
};

Paint offsat(const Paint&, const IntPoint& delta);
Paint offsat(const Paint&, const IntPoint& delta, const IntPoint& clickPos);

bool operator==(const Paint&, const Paint&);
bool operator!=(const Paint&, const Paint&);
bool operator==(const Paint&, const Color&);
bool operator==(const Color&, const Paint&);
bool operator!=(const Paint&, const Color&);
bool operator!=(const Color&, const Paint&);

// Get a color from the Paint if it contains a color, otherwise get
// the defaultColor
Color get_color_default(const Paint&, const Color& defaultColor);

// True if the Paint contains a Color with alpha of 255
bool is_opaque_color(const Paint&);

// True if the Paint contains a Color with alpha less than 255
bool is_translucent_color(const Paint&);

template<typename CF, typename PF, typename GF>
auto visit(const Paint& paint, CF colorFunc, PF patternFunc, GF gradientFunc)
  -> typename std::result_of<CF(Color)>::type
{
  if (paint.IsColor()){
    return colorFunc(paint.GetColor());
  }
  else if (paint.IsPattern()){
    return patternFunc(paint.GetPattern());
  }
  else {
    assert(paint.IsGradient());
    return gradientFunc(paint.GetGradient());
  }
}

} // namespace faint

#endif
