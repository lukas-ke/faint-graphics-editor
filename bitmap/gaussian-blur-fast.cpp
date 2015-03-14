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

#include <algorithm>
#include <cmath>
#include <vector>
#include "geo/primitive.hh"
#include "bitmap/channel.hh"
#include "bitmap/gaussian-blur.hh"

namespace faint{

// The fast gaussian blur implementation is modified from this
// javascript implementation:
//
// http://blog.ivank.net/fastest-gaussian-blur.html
// by Ivan Kuckir.

static std::vector<int> boxes_for_gauss(double sigma, int n){
  // Ideal averaging filter width
  const double wIdeal = std::sqrt((12*sigma*sigma/n)+1);
  int wl = floored(wIdeal);
  if (wl % 2 == 0){
    wl -= 1;
  }
  const int wu = wl + 2;
  const double mIdeal = (12*sigma*sigma - n*wl*wl - 4*n*wl - 3*n) /(-4*wl - 4);
  int m = rounded(mIdeal);
  std::vector<int> sizes;
  sizes.reserve(to_size_t(n));
  for (int i = 0; i != n; i++){
    sizes.push_back(i < m ? wl : wu);
  }
  return sizes;
}

static void box_blur_H(channel_t& scl, channel_t& tcl, const IntSize& size, int r){
  auto iarr = 1.0 / (r+r+1);
  for(int i = 0; i != size.h; i++){
    auto ti = i*size.w;
    auto li = ti;
    auto ri = ti+r;
    auto fv = scl[to_size_t(ti)];
    auto lv = scl[to_size_t(ti+size.w-1)];
    auto val = (r+1)*fv;

    for(int j=0; j<r; j++){
      val += scl[to_size_t(ti+j)];
    }
    for(int j=0; j<=r; j++){
      val += scl[to_size_t(ri++)] - fv;
      tcl[to_size_t(ti++)] = static_cast<unsigned char>(rounded(val*iarr));
    }
    for(int j=r+1; j<size.w-r; j++){
      val += scl[to_size_t(ri++)] - scl[to_size_t(li++)];
      tcl[to_size_t(ti++)] = static_cast<unsigned char>(rounded(val*iarr));
    }
    for(int j = size.w - r; j != size.w; j++){
      val += lv- scl[to_size_t(li++)];
      tcl[to_size_t(ti++)] = static_cast<unsigned char>(rounded(val*iarr));
    }
  }
}

static void box_blur_T(channel_t& scl, channel_t& tcl, const IntSize& size, int r){
  auto iarr = 1.0 / (r+r+1); // Fixme: ?
  for(int i=0; i != size.w; i++){
    auto ti = i;
    auto li = ti;
    auto ri = ti + r * size.w;
    auto fv = scl[to_size_t(ti)];
    auto lv = scl[to_size_t(ti+size.w * (size.h - 1))];
    auto val = (r + 1) * fv;
    for(int j = 0; j != r; j++){
      val += scl[to_size_t(ti + j * size.w)];
    }

    for(int j = 0; j <=r; j++){
      val += scl[to_size_t(ri)] - fv;
      tcl[to_size_t(ti)] = static_cast<unsigned char>(rounded(val*iarr));
      ri += size.w;
      ti += size.w;
    }

    for(int j = r + 1; j != size.h - r; j++){
      val += scl[to_size_t(ri)] - scl[to_size_t(li)];
      tcl[to_size_t(ti)] = static_cast<unsigned char>(rounded(val*iarr));
      li += size.w; 
      ri += size.w; 
      ti+=size.w;
    }

    for(int j = size.h - r; j != size.h; j++){
      val += lv - scl[to_size_t(li)];
      tcl[to_size_t(ti)] = static_cast<unsigned char>(rounded(val*iarr));
      li += size.w;
      ti += size.w;
    }
  }
}

static void box_blur(channel_t& scl, channel_t& tcl, const IntSize& size, int r){
  tcl = scl;
  box_blur_H(tcl, scl, size, r);
  box_blur_T(scl, tcl, size, r);
}

static void faux_gauss_blur(channel_t& scl, channel_t& tcl, const IntSize& size,
  const std::vector<int>& boxes)
{
  box_blur(scl, tcl, size, (boxes[0] - 1) / 2);
  box_blur(tcl, scl, size, (boxes[1] - 1) / 2);
  box_blur(scl, tcl, size, (boxes[2] - 1) / 2);
}

Bitmap gaussian_blur_fast(const Bitmap& bmp, double r){
  auto ch = separate_into_channels(bmp);
  auto ch2 = ch;
  const auto boxes = boxes_for_gauss(r, 3);
  faux_gauss_blur(ch.r, ch2.r, bmp.GetSize(), boxes);
  faux_gauss_blur(ch.g, ch2.g, bmp.GetSize(), boxes);
  faux_gauss_blur(ch.b, ch2.b, bmp.GetSize(), boxes);
  faux_gauss_blur(ch.a, ch2.a, bmp.GetSize(), boxes);
  return combine_into_bitmap(ch2);
}

} // namespace
