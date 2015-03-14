// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
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

#include <algorithm> // transform
#include <cassert>
#include <cmath> // sqrt
#include <iterator> // back_inserter
#include <numeric> // accumulate
#include <vector>
#include "util/math-constants.hh"
#include "bitmap/gaussian-blur.hh"

namespace faint{

static double gauss_1d(double x, double sigma){
  using faint::math::e;
  using faint::math::pi;
  return (1.0 / std::sqrt(2 * pi * sigma * sigma)) *
    pow(e, -((x*x) / (2 * sigma * sigma)));
}

// Normalizes the kernel so the pixel sum is 1 to avoid darkening the
// image
static std::vector<double> normalize(const std::vector<double>& k){
  double sum = std::accumulate(begin(k), end(k), 0.0);

  std::vector<double> k2;
  std::transform(begin(k), end(k), std::back_inserter(k2),
    [sum](double v){ return v / sum; });
  return k2;
}

static std::vector<double> gauss_kernel_1d(double sigma){
  std::vector<double> k;

  // Pixels further than 3*sigma have very little effect, limit kernel
  // size.
  int w = static_cast<int>(ceil(6 * sigma)) | 1;
  for (int x = 0; x != w; x++){
    k.push_back(gauss_1d(double(x - w / 2), sigma));
  }
  return k;
}

static std::vector<double> normalized_gauss_kernel_1d(double sigma){
  assert(sigma >= 0);
  return normalize(gauss_kernel_1d(sigma));
}

static Bitmap kernel_h_apply(const Bitmap& src, const std::vector<double>& k){
  Bitmap dst(src.GetSize());
  int r = static_cast<int>(k.size()) / 2;
  int w = resigned(k.size());
  const int bpp = 4;
  for (int y = 0; y != src.m_h; y++){
    const uchar* srcRow = src.m_data + y * src.m_row_stride;
    uchar* dstRow = dst.m_data + y * dst.m_row_stride;

    for (int x = 0; x != src.m_w; x++){
      double vR = 0;
      double vG = 0;
      double vB = 0;
      double vA = 0;

      for (int i = 0; i != w; i++){
        int d = std::min(x, std::max(i - r, -(src.m_w - x - 1)));
        vR += srcRow[(x - d) * bpp + iR] * k[to_size_t(i)];
        vG += srcRow[(x - d) * bpp + iG] * k[to_size_t(i)];
        vB += srcRow[(x - d) * bpp + iB] * k[to_size_t(i)];
        vA += srcRow[(x - d) * bpp + iA] * k[to_size_t(i)];
      }
      dstRow[x * BPP + iR] = static_cast<uchar>(vR);
      dstRow[x * BPP + iG] = static_cast<uchar>(vG);
      dstRow[x * BPP + iB] = static_cast<uchar>(vB);
      dstRow[x * BPP + iA] = static_cast<uchar>(vA);
    }
  }
  return dst;
}

static Bitmap kernel_v_apply(const Bitmap& src, const std::vector<double>& k){
  Bitmap dst(src.GetSize());
  int r = static_cast<int>(k.size()) / 2;
  int w = resigned(k.size());
  static_assert(BPP == 4, "4-bytes per pixel required.");
  for (int y = 0; y != src.m_h; y++){
    const uchar* srcRow = src.m_data + y * src.m_row_stride;
    uchar* dstRow = dst.m_data + y * dst.m_row_stride;
    for (int x = 0; x != src.m_w; x++){
      double vR = 0;
      double vG = 0;
      double vB = 0;
      double vA = 0;
      for (int i = 0; i != w; i++){
        const int d = std::min(y, std::max(i - r, -(src.m_h - y - 1)));
        vR += srcRow[x * BPP - d * src.m_row_stride + iR] * k[to_size_t(i)];
        vG += srcRow[x * BPP - d * src.m_row_stride + iG] * k[to_size_t(i)];
        vB += srcRow[x * BPP - d * src.m_row_stride + iB] * k[to_size_t(i)];
        vA += srcRow[x * BPP - d * src.m_row_stride + iA] * k[to_size_t(i)];
      }

      dstRow[x * BPP + iR] = static_cast<uchar>(vR);
      dstRow[x * BPP + iG] = static_cast<uchar>(vG);
      dstRow[x * BPP + iB] = static_cast<uchar>(vB);
      dstRow[x * BPP + iA] = static_cast<uchar>(vA);
    }
  }
  return dst;
}

Bitmap gaussian_blur_exact(const Bitmap& src, double sigma){
  auto k = normalized_gauss_kernel_1d(sigma);
  return kernel_v_apply(kernel_h_apply(src, k), k);
}

} // namespace
