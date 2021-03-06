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
#include "bitmap/scale-nearest.hh"

namespace faint{

Bitmap scale_nearest(const Bitmap& src, int scale){
  assert(scale > 0);
  return scale_nearest(src, src.GetSize() * scale);
}

Bitmap scale_nearest(const Bitmap& src, const IntSize& newSize){
  assert(newSize.w > 0 && newSize.h > 0);
  const int w2 = newSize.w;
  const int h2 = newSize.h;

  Bitmap scaled(IntSize(w2, h2));
  int x_ratio = (src.m_w << 16) / scaled.m_w + 1;
  int y_ratio = (src.m_h << 16) / scaled.m_h + 1;
  uchar* p_dst = scaled.m_data;
  const uchar* p_src = src.m_data;

  for (int j = 0; j < h2; j++){
    for (int i = 0; i < w2; i++){
      int x2 = ((i*x_ratio)>>16);
      int y2 = ((j*y_ratio)>>16);

      uchar* rDst = p_dst + j * (scaled.m_row_stride) + i * ByPP;
      const uchar* rSrc = p_src + y2 * (src.m_row_stride) + x2 * ByPP;

      *(rDst + 0) = *rSrc;
      *(rDst + 1) = *(rSrc + 1);
      *(rDst + 2) = *(rSrc + 2);
      *(rDst + 3) = *(rSrc + 3);
    }
  }
  return scaled;
}

} // namespace
