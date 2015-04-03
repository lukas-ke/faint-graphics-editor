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

#include "bitmap/bitmap.hh"
#include "bitmap/bitmap-exception.hh"
#include "bitmap/color.hh"
#include "bitmap/mask.hh"

namespace faint{

Mask::Mask(const IntSize& size)
  : m_size(size)
{
  if (size.w <= 0){
    throw BitmapException("Invalid width (mask)");
  }
  else if (size.h <= 0){
    throw BitmapException("Invalid height (mask)");
  }

  try{
    m_data = new bool[size.w * size.h];
  }
  catch (const std::bad_alloc&){
    throw BitmapOutOfMemory("mask");
  }
}

Mask::~Mask(){
  delete[] m_data;
  m_data = nullptr;
}

void Mask::Set(int x, int y, bool value){
  if (0 <= x && x < m_size.w && 0 <= y && y < m_size.h){
    m_data[m_size.w * y + x] = value;
  }
}

bool Mask::Get(int x, int y) const{
  if (0 <= x && x < m_size.w && 0 <= y && y < m_size.h){
    return m_data[m_size.w * y + x];
  }
  return false;
}

Mask mask_set_color(const Bitmap& bmp, const Color& c){
  const auto size = bmp.GetSize();
  Mask m(size);
  for (int y = 0; y != size.h; y++){
    for (int x = 0; x != size.w; x++){
      m.Set(x, y, get_color_raw(bmp, x, y) == c);
    }
  }
  return m;
}

Mask mask_not_color(const Bitmap& bmp, const Color& c){
  const auto size = bmp.GetSize();
  Mask m(size);
  for (int y = 0; y != size.h; y++){
    for (int x = 0; x != size.w; x++){
      m.Set(x, y, get_color_raw(bmp, x, y) != c);
    }
  }
  return m;
}

} // namespace
