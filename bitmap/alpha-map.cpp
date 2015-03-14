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

#include <algorithm>
#include <cassert>
#include <cstring> // memcpy
#include "bitmap/alpha-map.hh"
#include "bitmap/brush.hh"
#include "geo/int-point.hh"
#include "geo/int-rect.hh"
#include "util/optional.hh"

namespace faint{

inline bool valid_pos(int x, int y, const IntSize& size){
  return 0 <= x && x < size.w && 0 <= y && y < size.h;
}

inline int to_index(int x, int y, int stride){
  return x + y * stride;
}

AlphaMapRef::AlphaMapRef(const uchar* data, const IntSize& size, int stride)
  : m_data(data),
    m_size(size),
    m_stride(stride)
{}

uchar AlphaMapRef::Get(int x, int y) const{
  return m_data[to_index(x,y,m_stride)];
}

IntSize AlphaMapRef::GetSize() const{
  return m_size;
}

Optional<IntRect> AlphaMapRef::BoundingRect() const{
  int minX = m_size.w;
  int minY = m_size.h;
  int maxX = 0;
  int maxY = 0;
  for (int y = 0; y != m_size.h; y++){
    for (int x = 0; x != m_size.w; x++){
      if (Get(x,y) != 0){
        minX = std::min(minX, x);
        minY = std::min(minY, y);
        maxX = std::max(maxX, x);
        maxY = std::max(maxY, y);
      }
    }
  }
  if (minX <= maxX && minY <= maxY){
    return IntRect(IntPoint(minX, minY), IntPoint(maxX, maxY));
  }
  return {};
}


AlphaMap::AlphaMap(const IntSize& sz)
  : m_data(nullptr),
    m_stride(0)
{
  Initialize(sz);
}

AlphaMap::AlphaMap(const AlphaMap& other)
  : m_size(other.m_size),
    m_stride(other.m_stride)
{
  assert(m_size.w > 0 && m_size.h > 0);
  size_t length = to_size_t(area(m_size));
  m_data = new uchar[length];
  memcpy(m_data, other.m_data, length);
}

AlphaMap::~AlphaMap(){
  delete[] m_data;
}

void AlphaMap::Add(int x, int y, uchar value){
  uchar& item = m_data[to_index(x, y, m_stride)];
  item = static_cast<uchar>(std::min(int(item) + value, 255));
}

void AlphaMap::Set(int x, int y, uchar value){
  if (valid_pos(x, y, m_size)){
    m_data[to_index(x, y, m_stride)] = value;
  }
}

void AlphaMap::Clear(){
  memset(m_data, 0, to_size_t(area(m_size)));
}

AlphaMapRef AlphaMap::FullReference() const{
  return AlphaMapRef(m_data, m_size, m_stride);
}

uchar AlphaMap::Get(int x, int y) const {
  return m_data[to_index(x,y, m_stride)];
}

uchar* AlphaMap::GetRaw(){
  return m_data;
}

const uchar* AlphaMap::GetRaw() const{
  return m_data;
}

IntSize AlphaMap::GetSize() const{
  return m_size;
}

void AlphaMap::Initialize(const IntSize& sz){
  assert(sz.w > 0 && sz.h > 0);
  m_size = sz;
  m_data = new uchar[to_size_t(area(m_size))];
  m_stride = sz.w;
  Clear();
}

void AlphaMap::Reset(const IntSize& size){
  if (GetSize() != size){
    delete[] m_data;
    Initialize(size);
  }
  else{
    Clear();
  }
}

AlphaMap AlphaMap::SubCopy(const IntRect& r) const{
  assert(0 <= r.x && 0 <= r.y &&
    r.x + r.w <= m_size.w &&
    r.y + r.h <= m_size.h);

  AlphaMap a(r.GetSize());
  for (int y = 0; y != r.h; y++){
    for (int x = 0; x != r.w; x++){
      a.Set(x,y,Get(x + r.x,y + r.y));
    }
  }
  return a;
}

AlphaMapRef AlphaMap::SubReference(const IntRect& r) const{
  assert(0 <= r.x && 0 <= r.y &&
    r.x + r.w <= m_size.w &&
    r.y + r.h <= m_size.h);
  return AlphaMapRef(m_data +
    to_index(r.x, r.y, m_stride), r.GetSize(), m_stride);
}

static void brush_stroke(AlphaMap& data, int x, int y, const Brush& b){
  IntSize brushSize(b.GetSize());
  IntSize dataSize(data.GetSize());
  int xCenter = 0;
  int yCenter = 0;
  for (int yB = 0; yB != brushSize.h; yB++){
    for (int xB = 0; xB != brushSize.w; xB++){
      int xD = x + xB - xCenter;
      int yD = y + yB - yCenter;
      if (xD < 0 || yD < 0 || dataSize.w <= xD || dataSize.h <= yD){
        continue;
      }

      data.Add(x + xB - xCenter, y + yB - yCenter, b.Get(xB, yB));
    }
  }
}

static int int_abs(int v){
  return v < 0 ? -v : v;
}

using std::swap;
void stroke(AlphaMap& data, const UpperLeft& p0, const UpperLeft& p1,
  const Brush& b)
{

int x0 = p0.Get().x;
  int y0 = p0.Get().y;
  int x1 = p1.Get().x;
  int y1 = p1.Get().y;
  const bool steep = int_abs(y1 - y0) > int_abs(x1 - x0);
  if (steep){
    swap(x0, y0);
    swap(x1, y1);
  }
  if (x0 > x1){
    swap(x0, x1);
    swap(y0, y1);
  }

  int dx = x1 - x0;
  int dy = int_abs(y1 - y0);
  int err = 0;
  int y = y0;
  int yStep = y0 < y1 ? 1 : -1;
  for (int x = x0; x <= x1; x++){
    err += dy;
    if (steep){
      brush_stroke(data, y, x, b);
    }
    else {
      brush_stroke(data, x, y, b);
    }

    if (2 * err > dx){
      y += yStep;
      err -= dx;
    }
  }
}

} // namespace
