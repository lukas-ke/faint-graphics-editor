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

#include <cassert>
#include "bitmap/alpha-map.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/color-list.hh"

namespace faint{

ColorList::ColorList(){
  m_colors.reserve(256);
}

void ColorList::AddColor(const Color& color){
  m_colors.push_back(color);
}

Color ColorList::GetColor(int index) const{
  assert(index < resigned(m_colors.size()));
  return m_colors[to_size_t(index)];
}

int ColorList::GetNumColors() const{
  return resigned(m_colors.size());
}

Bitmap bitmap_from_indexed_colors(const AlphaMap& alphaMap,
  const ColorList& colorMap)
{
  const IntSize sz(alphaMap.GetSize());
  Bitmap dst(sz);
  for (int y = 0; y != sz.h; y++){
    for (int x = 0; x != sz.w; x++){
      uchar index = alphaMap.Get(x,y);
      Color color = colorMap.GetColor(index);
      put_pixel_raw(dst, x, y, color);
    }
  }
  return dst;
}

} // namespace
