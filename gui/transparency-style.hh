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

#ifndef FAINT_TRANSPARENCY_STYLE_HH
#define FAINT_TRANSPARENCY_STYLE_HH
#include "bitmap/color.hh"
#include "util/optional.hh"

namespace faint{

class TransparencyStyle{
  // The background style for indicating image regions with alpha.
public:
  // Default (Blend towards checkered pattern)
  TransparencyStyle();

  // Blend towards solid color
  TransparencyStyle(const ColRGB&);

  bool IsCheckered() const;
  bool IsColor() const;

  // Note: IsColor must be true.
  ColRGB GetColor() const;
private:
  Optional<ColRGB> m_color;
};

} // namespace

#endif
