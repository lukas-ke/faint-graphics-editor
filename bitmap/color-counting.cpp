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

#include <algorithm>
#include <cassert>
#include <unordered_set>
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/color-counting.hh"

namespace faint{

static void insert_color_count(color_counts_t& colors, const Color& c){
  auto it = colors.find(c);
  if (it != colors.end()){
    it->second++;
  }
  else{
    colors.insert(std::make_pair(c, 1));
  }
}

void add_color_counts(const Bitmap& bmp, color_counts_t& colors){
  assert(bmp.m_w > 0 && bmp.m_h > 0);
  for (int y = 0; y != bmp.m_h; y++){
    for (int x = 0; x != bmp.m_w; ++x){
      insert_color_count(colors, get_color_raw(bmp, x,y));
    }
  }
}

Color most_common(const color_counts_t& colors){
  assert(!colors.empty());
  using count_t = color_counts_t::value_type;
  auto it = std::max_element(begin(colors), end(colors),
    [](const count_t& c1, const count_t& c2){return c1.second < c2.second;});
  assert(it != colors.end());
  return it->first;
}

int count_colors(const Bitmap& bmp){
  std::unordered_set<unsigned int> s;
  for (int y = 0; y != bmp.m_h; y++){
    for (int x = 0; x != bmp.m_w; x++){
      s.insert(to_hash(get_color_raw(bmp, x,y)));
    }
  }
  return resigned(s.size());
}

} // namespace
