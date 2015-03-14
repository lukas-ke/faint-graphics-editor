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

#include "bitmap/bitmap.hh"
#include "bitmap/channel.hh"
#include "bitmap/color.hh"
#include "bitmap/iter-bmp.hh"

namespace faint{

Channels separate_into_channels(const Bitmap& bmp){
  Channels channels;
  const size_t length = to_size_t(area(bmp.GetSize()));
  channels.r.reserve(length);
  channels.g.reserve(length);
  channels.b.reserve(length);
  channels.a.reserve(length);

  channels.w = bmp.m_w;

  const uchar* data = bmp.GetRaw();

  for (ITER_XY_OFFSET(x, y, i, bmp)){
    channels.r.push_back(data[i + iR]);
    channels.g.push_back(data[i + iG]);
    channels.b.push_back(data[i + iB]);
    channels.a.push_back(data[i + iA]);
  }

  return channels;
}

Bitmap combine_into_bitmap(const Channels& channels){
  const int w = channels.w;
  const int h = resigned(channels.r.size()) / w;

  Bitmap bmp(IntSize(channels.w, h));

  uchar* data = bmp.GetRaw();
  for (ITER_XY_OFFSET(x, y, iBmp, bmp)){
    const size_t iCh = static_cast<size_t>(y * w + x);
    data[iBmp + iR] = channels.r[iCh];
    data[iBmp + iG] = channels.g[iCh];
    data[iBmp + iB] = channels.b[iCh];
    data[iBmp + iA] = channels.a[iCh];
  }
  return bmp;
}

Color get_color(const Channels& ch, int index){
  const size_t i(static_cast<size_t>(index));
  return Color(ch.r[i],
    ch.g[i],
    ch.b[i],
    ch.a[i]);
}

std::vector<Color> get_colors(const Channels& ch){
  std::vector<Color> colors;
  colors.reserve(ch.r.size());

  const int sz = resigned(ch.r.size());
  for (int i = 0; i != sz; i++){
    colors.emplace_back(get_color(ch, i));
  }

  return colors;
}

} // namespace
