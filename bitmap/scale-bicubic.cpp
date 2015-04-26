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

#include <array>
#include <cassert>
#include <vector>
#include "bitmap/color.hh"
#include "bitmap/scale-bicubic.hh"
#include "geo/geo-func.hh"
#include "geo/int-size.hh"

namespace faint{

// Cubic interpolation uses four samples per dimension.
const int OFFSET_BEGIN = -1;
const int OFFSET_END = 3;
const int OFFSET_STEPS = OFFSET_END - OFFSET_BEGIN;
static_assert(OFFSET_STEPS == 4, "Expected 4 steps");

inline coord R(coord v){
  // Cubic weighing function which gives the interpolated value
  // Source: http://paulbourke.net/texture_colour/imageprocess/
  // <../doc/bicubic_R.png>

  const auto cc = [](coord v){
    return v > 0 ? v*v*v : 0;
  };

  return (cc(v + 2) -
    4 * cc(v + 1) +
    6 * cc(v) -
    4 * cc(v - 1)) / 6;
}

inline coord src_coord_subpixel(int dst, int srcSize, int dstSize){
  // Returns the "exact" source coordinate for this target coordinate
  // (dst), when the valid span for dst is dstSize, and the span of
  // source is srcSize.
  return dst * (srcSize - 1) / (coord)(dstSize - 1);
}

static auto all_column_weights(const int srcWidth, const int dstWidth){
  // Compute the weights for the four closest x-coordinates for
  // the coordinates in the source-image that correspond to each
  // destination x-coordinate.

  std::vector<coord> columnWeights(dstWidth * OFFSET_STEPS);

  for (int i = 0; i != dstWidth ; i++){
    const coord x = src_coord_subpixel(i, srcWidth, dstWidth);
    const coord dx = x - (int)x;

    for (int m = OFFSET_BEGIN; m != OFFSET_END; m++){
      const int index = m + 1 + i * OFFSET_STEPS;
      assert(0 <= index && index < OFFSET_STEPS * dstWidth);
      columnWeights[index] = R(m - dx);
    }
  }
  return columnWeights;
}

static auto current_row_weights(coord dy){
  // Compute the weights for the four rows surrounding a source-row
  // with the given fractional part of the y-coordinate (dy).
  std::array<coord, 4> nWeight{{0,0,0,0}};

  for (int n = OFFSET_BEGIN; n != OFFSET_END; n++){
    nWeight[n + 1] = R(dy - n);
  }
  return nWeight;
}

Bitmap scale_bicubic(const Bitmap& src, const Scale& scale){
  // Bicubic interpolation estimates the destination pixel colors by
  // considering the 16 surrounding pixels in the source image.
  //
  // The coordinates (xi, yi) are the nearest integer
  // pixel-coordinates in the source image for pixel coordinates (i,
  // j) in the target image. The difference between (xi, yi) and the
  // exact "subpixel" coordinate is given by (dx, dy).
  //
  // <../doc/bicubic_position.png>
  //
  // This formula gives the interpolated value to apply to each color
  // channel:
  // <../doc/bicubic_F.png>
  //
  // Source:
  // http://paulbourke.net/texture_colour/imageprocess/

  const auto srcSize = src.GetSize();
  const auto dstSize = rounded(floated(srcSize) * scale);
  assert(srcSize.w > 0 && srcSize.h > 0);
  assert(dstSize.w > 0 && dstSize.h > 0); // Fixme: Clamp instead?

  Bitmap dst(dstSize);
  const auto dstStride = dst.GetStride();
  assert(bitmap_ok(dst));

  // Precompute the column weights (4 per column)
  const auto mWeights = all_column_weights(srcSize.w, dstSize.w);

  // For each destination row (j)
  for (int j = 0; j != dstSize.h; j++){
    uchar* pDst = dst.GetRaw() + j * dstStride;
    const coord y = src_coord_subpixel(j, srcSize.h, dstSize.h);

    // Integer y-coordinate in source image
    const int yi = (int)y;

    // The four row weights for the current row
    const auto nWeight = current_row_weights(y - yi);

    // For each destination column (i)
    for (int i = 0; i != dstSize.w; i++){

      // Integer x-coordinate in source image
      const int xi = (int)src_coord_subpixel(i, srcSize.w, dstSize.w);
      assert(0 <= xi && xi < src.m_w);
      assert(0 <= yi && yi < src.m_h);

      // Summed color values over n*m
      coord r = 0;
      coord g = 0;
      coord b = 0;
      coord a = 0;
      coord totalWeight = 0;

      // Fixme: Move into inner loop.
      const uchar* pSrc = src.GetRaw() + (yi - 1) * src.GetStride() +
        (xi - 1) * ByPP;

      // For each weight combination, row (=n) by column (=m)
      for (int n = OFFSET_BEGIN; n != OFFSET_END; n++){
        for (int m = OFFSET_BEGIN; m != OFFSET_END; m++){
          if (within_bitmap(src, xi + m, yi + n)){
            const auto w =
              mWeights[m + 1 + OFFSET_STEPS * i] *
              nWeight[n + 1];

            assert(src.GetRaw() <= pSrc);
            assert(pSrc < src.GetRaw() + src.m_h * src.GetStride());

            if (pSrc < src.GetRaw() + src.m_h * src.GetStride()){
              b += pSrc[iB] * w * pSrc[iA];
              g += pSrc[iG] * w * pSrc[iA];
              r += pSrc[iR] * w * pSrc[iA];
              a += pSrc[iA] * w;
              totalWeight += w;
            }
          }
          pSrc += ByPP;
        }
        pSrc = pSrc - ByPP * OFFSET_STEPS + src.GetStride();
      }

      static const auto sum_to_color =
        [](coord colorSum, coord alphaSum) -> uchar{
          return alphaSum == 0 ? 0 : static_cast<uchar>(colorSum / alphaSum + 0.5);
        };

      pDst[iR] = sum_to_color(r, a);
      pDst[iG] = sum_to_color(g, a);
      pDst[iB] = sum_to_color(b, a);
      pDst[iA] = static_cast<uchar>(a / totalWeight + 0.5);
      pDst += ByPP;
    }
  }
  return dst;
}

} // namespace
