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

#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "geo/rotation-adjustment.hh"

// The rotation code is adapted from this example for Windows GDI by
// Yves Maurer: http://www.codeguru.com/cpp/g-m/gdi/article.php/c3693
namespace faint {

Bitmap rotate_nearest(const Bitmap& src,
  const Angle& angle,
  const Color& bgColor)
{
  RotationAdjustment adj = get_rotation_adjustment(angle,
    src.GetSize());

  Bitmap dst(adj.size, bgColor);
  uchar* dstRow = dst.m_data;

  coord ca = cos(angle);
  coord sa = sin(angle);
  coord divisor = ca*ca + sa*sa;

  // Use the upper-left corner as pivot
  const double CtX = 0;
  const double CtY = 0;

  const uchar* pSrc = src.m_data;

  // Iterate over the destination bitmap
  for (int j = 0; j != dst.m_h; j++){
    for (int i = 0; i != dst.m_w; i++){

      // Calculate the source coordinate
      int x = static_cast<int>((ca * (((double) i + adj.offset.x) +
            CtX * (ca - 1)) +
          sa * (((double) j + adj.offset.y) +
            CtY * (sa - 1))) / divisor);

      int y = static_cast<int>(CtY + (CtX - ((double) i + adj.offset.x)) *
        sa + ca *(((double) j + adj.offset.y) - CtY + (CtY - CtX) * sa));

      if (0 <= x && x < src.m_w && 0 <= y && y < src.m_h){
        // Inside source - copy the bits
        dstRow[i * ByPP + iR] = pSrc[x*ByPP + y * src.m_row_stride + iR];
        dstRow[i * ByPP + iG] = pSrc[x*ByPP + y * src.m_row_stride + iG];
        dstRow[i * ByPP + iB] = pSrc[x*ByPP + y * src.m_row_stride + iB];
        dstRow[i * ByPP + iA] = pSrc[x*ByPP + y * src.m_row_stride + iA];
      }
      else {
        // Outside source - set the color to bg color
        dstRow[i * ByPP + iR] = bgColor.r;
        dstRow[i * ByPP + iG] = bgColor.g;
        dstRow[i * ByPP + iB] = bgColor.b;
        dstRow[i * ByPP + iA] = bgColor.a;
      }
    }
    dstRow += dst.m_row_stride;
  }

  return dst;
}

} // namespace
