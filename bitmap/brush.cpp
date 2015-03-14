// -*- coding: us-ascii-unix -*-
// Copyright 2013 Lukas Kemmer
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

#include <cassert>
#include <cstring> // memset
#include "bitmap/brush.hh"
#include "geo/int-point.hh"
#include "geo/int-size.hh"

namespace faint{

Brush::Brush(const IntSize& sz)
  : m_w(sz.w),
    m_h(sz.h)
{
  assert(m_w > 0);
  assert(m_h > 0);
  m_data = new uchar[to_size_t(m_w * m_h)];
  memset(m_data, 0, to_size_t(m_w * m_h));
}

Brush::Brush(const Brush& other){
  m_w = other.m_w;
  m_h = other.m_h;
  m_data = new uchar[to_size_t(m_w * m_h)];
  memcpy(m_data, other.m_data, to_size_t(m_w * m_h));
}

Brush::~Brush(){
  delete[] m_data;
}

void Brush::Set(int x, int y, uchar value){
  m_data[ y * m_w + x ] = value;
}

void Brush::Set(const IntPoint& pos, uchar value){
  Set(pos.x, pos.y, value);
}

uchar Brush::Get(int x, int y) const{
  assert(x >= 0 && y >= 0 && x < m_w && y < m_h);
  return m_data[ y * m_w + x ];
}

IntSize Brush::GetSize() const{
  return IntSize(m_w, m_h);
}

Brush& Brush::operator=(const Brush& other){
  if (&other == this){
    return *this;
  }
  m_w = other.m_w;
  m_h = other.m_h;
  delete[] m_data;
  m_data = new uchar[to_size_t(m_w * m_h)];
  memcpy(m_data, other.m_data, to_size_t(other.m_w * other.m_h));
  return *this;
}

static Brush circle_brush(int w, uchar alpha){
  if (w <= 1){
    Brush b(IntSize::Both(1));
    b.Set(0,0,alpha);
    return b;
  }

  int rd = w / 2;

  int ofs = w % 2 == 0 ? 1 : 0;

  int f = 1 - rd;
  int ddF_x = 1;
  int ddF_y = -2 * rd;
  int x = 0;
  int y = rd;

  Brush b(IntSize::Both(w));

  int cx = rd;
  int cy = rd;
  b.Set(cx, cy + rd - ofs, alpha);
  b.Set(cx, cy - rd, alpha);
  b.Set(cx + rd - ofs, cy, alpha);
  b.Set(cx - rd, cy, alpha);

  while(x <= y) {
    if(f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;
    for (int i = 0; i <= x; i++){
      b.Set(cx + i - ofs, cy + y - ofs, alpha);
      b.Set(cx - i, cy + y - ofs, alpha);
      b.Set(cx + i - ofs, cy - y, alpha);
      b.Set(cx - i, cy - y, alpha);
    }

    for (int i = 0; i <= y; i++){
      b.Set(cx + i - ofs, cy + x - ofs, alpha);
      b.Set(cx - i, cy + x - ofs, alpha);
      b.Set(cx + i - ofs, cy - x, alpha);
      b.Set(cx - i, cy - x, alpha);
    }
  }
  for (int i = 0; i != rd; i++){
    b.Set(cx - i, cy, alpha);
    b.Set(cx + i - ofs, cy, alpha);
  }
  return b;
}

Brush circle_brush(int w){
  return circle_brush(w, 255);
}

Brush rect_brush(int sz){
  if (sz <= 1){
    Brush b(IntSize::Both(1));
    b.Set(0,0, 255);
    return b;
  }

  Brush b(IntSize::Both(sz));
  for (int y = 0; y != sz; y++){
    for (int x = 0; x != sz; x++){
      b.Set(x, y, 255);
    }
  }
  return b;
}

Brush experimental_brush(int w){
  return circle_brush(w, 1);
}

} // namespace
