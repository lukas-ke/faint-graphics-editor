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
#include "bitmap/rotate-util.hh"
#include "geo/angle.hh"

// The rotation code is adapted from this example for Windows GDI by
// Yves Maurer: http://www.codeguru.com/cpp/g-m/gdi/article.php/c3693
namespace faint {

static double min4(double a, double b, double c, double d){
  if (a < b){
    if (c < a){
      return (d < c) ? d : c;
    }
    else {
      return (d < a) ? d : a;
    }
  }
  else if (c < b){
    return (d < c) ? d : c;
  }
  else {
    return (d < b) ? d : b;
  }
}

static double max4(double a, double b, double c, double d){
  if (a > b){
    if (c > a){
      return (d > c) ? d : c;
    }
    else {
      return (d > a) ? d : a;
    }
  }
  else if (c > b){
    return (d > c) ? d : c;
  }
  else {
    return (d > b) ? d : b;
  }
}

RotationAdjustment::RotationAdjustment(const IntPoint& offset,
  const IntSize& size)
  : offset(offset),
    size(size)
{}

RotationAdjustment get_rotation_adjustment(const Angle& angle,
  const IntSize& size)
{
  // Use the upper-left corner as pivot
  const coord CtX = 0;
  const coord CtY = 0;

  // Find the corners to initialize the destination width and height
  const coord cA = cos(angle);
  const coord sA = sin(angle);

  const coord x1 = CtX + (-CtX) * cA - (-CtY) * sA;
  const coord x2 = CtX + (size.w - CtX) * cA - (-CtY) * sA;
  const coord x3 = CtX + (size.w - CtX) * cA - (size.h - CtY) * sA;
  const coord x4 = CtX + (-CtX) * cA - (size.h - CtY) * sA;

  const coord y1 = CtY + (-CtY) * cA + (-CtX) * sA;
  const coord y2 = CtY + (size.h - CtY) * cA + (-CtX) * sA;
  const coord y3 = CtY + (size.h - CtY) * cA + (size.w - CtX) * sA;
  const coord y4 = CtY + (-CtY) * cA + (size.w - CtX) * sA;

  IntPoint offset(floored(min4(x1, x2, x3, x4)),
    floored(min4(y1, y2, y3, y4)));
  IntSize newSize(ceiled(max4(x1, x2, x3, x4)) - offset.x,
    ceiled(max4(y1,y2,y3,y4)) - offset.y);

  return RotationAdjustment(offset, newSize);
}


Bitmap rotate_nearest(const Bitmap& bmp,
  const Angle& angle,
  const Color& bgColor)
{
  // Use the upper-left corner as pivot
  double CtX = 0;
  double CtY = 0;

  // Find the corners to initialize the destination width and height

  RotationAdjustment adj = get_rotation_adjustment(angle,
    bmp.GetSize());

  Bitmap bmpDst(adj.size, bgColor);
  uchar* dstRow = bmpDst.m_data;

  const uchar* src = bmp.m_data;
  coord ca = cos(angle);
  coord sa = sin(angle);
  coord divisor = ca*ca + sa*sa;

  // Iterate over the destination bitmap
  for (int stepY = 0; stepY != bmpDst.m_h; stepY++){
    for (int stepX = 0; stepX != bmpDst.m_w; stepX++){
      // Calculate the source coordinate
      int orgX = static_cast<int>((ca * (((double) stepX + adj.offset.x) +
            CtX * (ca - 1)) +
          sa * (((double) stepY + adj.offset.y) +
            CtY * (sa - 1))) / divisor);

      int orgY = static_cast<int>(CtY + (CtX - ((double) stepX + adj.offset.x)) *
        sa + ca *(((double) stepY + adj.offset.y) - CtY + (CtY - CtX) * sa));

      if (orgX >= 0 && orgY >= 0 && orgX < bmp.m_w && orgY < bmp.m_h){
        // Inside source - copy the bits
        dstRow[stepX * BPP + iR] = src[orgX * BPP + orgY * bmp.m_row_stride + iR];
        dstRow[stepX * BPP + iG] = src[orgX * BPP + orgY * bmp.m_row_stride + iG];
        dstRow[stepX * BPP + iB] = src[orgX * BPP + orgY * bmp.m_row_stride + iB];
        dstRow[stepX * BPP + iA] = src[orgX * BPP + orgY * bmp.m_row_stride + iA];
      }
      else {
        // Outside source - set the color to bg color
        dstRow[stepX * BPP + iR] = bgColor.r;
        dstRow[stepX * BPP + iG] = bgColor.g;
        dstRow[stepX * BPP + iB] = bgColor.b;
        dstRow[stepX * BPP + iA] = bgColor.a;
      }
    }
    dstRow += bmpDst.m_row_stride;
  }

  return bmpDst;
}

} // namespace
