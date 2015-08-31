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

#include "bitmap/color-ptr.hh"
#include "bitmap/draw.hh" // For flip
#include "bitmap/scale-bilinear.hh"
#include "geo/axis.hh" // For flip
#include "geo/geo-func.hh"
#include "geo/range.hh" // constrained
#include "geo/scale.hh"

namespace faint{

Bitmap scale_bilinear(const Bitmap& src, const Scale& scale){
  IntSize newSize(constrained(rounded(floated(src.GetSize()) * abs(scale)),
      min_t(1), min_t(1)));
  if (newSize == src.GetSize()){
    return Bitmap(src);
  }

  Bitmap dst(newSize);
  const coord x_ratio = floated(src.m_w - 1) / floated(newSize.w);
  const coord y_ratio = floated(src.m_h - 1) / floated(newSize.h);
  const uchar* data = src.m_data;

  for (int j = 0; j != newSize.h; j++){
    for (int i = 0; i != newSize.w; i++){
      int xSrc = truncated(x_ratio * i);
      int ySrc = truncated(y_ratio * j);
      const coord x_diff = x_ratio * i - xSrc;
      const coord y_diff = y_ratio * j - ySrc;
      const int srcIndex = ySrc * src.m_row_stride + xSrc * ByPP;
      const_color_ptr a(data + srcIndex);
      const_color_ptr b(data + srcIndex + ByPP);
      const_color_ptr c(data + srcIndex + src.m_row_stride);
      const_color_ptr d(data + srcIndex + src.m_row_stride + ByPP);

      const uchar blue = static_cast<uchar>((a.b)*(1-x_diff)*(1-y_diff) +
        (b.b)*(x_diff)*(1-y_diff) +
        (c.b)*(y_diff)*(1-x_diff) +
        (d.b)*(x_diff*y_diff) + 0.5);

      const uchar green = static_cast<uchar>(a.g*(1-x_diff)*(1-y_diff) +
        b.g*(x_diff)*(1-y_diff) +
        (c.g)*(y_diff)*(1-x_diff) +
        (d.g)*(x_diff*y_diff) + 0.5);

      const uchar red = static_cast<uchar>(a.r * (1-x_diff)*(1-y_diff) +
        b.r*(x_diff)*(1-y_diff) +
        c.r*(y_diff)*(1-x_diff) +
        (d.r)*(x_diff*y_diff) + 0.5);

      const uchar alpha = static_cast<uchar>(a.a * (1-x_diff)*(1-y_diff) +
        b.a*(x_diff)*(1-y_diff) +
        c.a*(y_diff)*(1-x_diff) +
        (d.a)*(x_diff*y_diff) + 0.5);

      uchar* rDst = dst.m_data + j * (dst.m_row_stride) + i * ByPP;
      *(rDst + iR) = red;
      *(rDst + iG) = green;
      *(rDst + iB) = blue;
      *(rDst + iA) = alpha;
    }
  }

  if (scale.x < 0){
    dst = flip(dst, across(Axis::VERTICAL));
  }
  if (scale.y < 0) {
    dst = flip(dst, across(Axis::HORIZONTAL));
  }
  return dst;
}

} // namespace
