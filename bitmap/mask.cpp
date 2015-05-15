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
#include <cstring> // memcpy
#include "bitmap/bitmap.hh"
#include "bitmap/bitmap-exception.hh"
#include "bitmap/color.hh"
#include "bitmap/mask.hh"

namespace faint{

auto* allocate_mask_data(const IntSize& size){
  if (size.w <= 0){
    throw BitmapException("Invalid width (mask)");
  }
  else if (size.h <= 0){
    throw BitmapException("Invalid height (mask)");
  }

  try{
    return new bool[area(size)];
  }
  catch (const std::bad_alloc&){
    throw BitmapOutOfMemory("mask");
  }
}

Mask::Mask(const IntSize& size)
  : m_size(size),
    m_data(allocate_mask_data(size))
{}

Mask::Mask(Mask&& moved)
  : m_size(moved.m_size),
    m_data(moved.m_data)
{
  moved.m_data = nullptr;
}

Mask::Mask(const Mask& other)
  : m_size(other.m_size),
    m_data(allocate_mask_data(other.m_size))
{
  memcpy(m_data, other.m_data, area(m_size));
}

Mask::~Mask(){
  delete[] m_data;
}

bool Mask::Any() const{
  int len = area(m_size);
  for (int i = 0; i != len; i++){
    if (m_data[i]){
      return true;
    }
  }
  return false;
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

IntSize Mask::GetSize() const{
  return m_size;
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

Mask mask_alpha_equal(const Bitmap& bmp, uchar maskAlpha){
  const auto size = bmp.GetSize();
  Mask m(size);
  for (int y = 0; y != size.h; y++){
    for (int x = 0; x != size.w; x++){
      m.Set(x, y, get_color_raw(bmp, x, y).a == maskAlpha);
    }
  }
  return m;
}

} // namespace
