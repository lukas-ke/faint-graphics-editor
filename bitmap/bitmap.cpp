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

#include <cassert>
#include <cstring> // memcpy
#include <limits>
#include "bitmap/bitmap.hh"
#include "bitmap/bitmap-exception.hh"
#include "bitmap/color.hh"
#include "bitmap/draw.hh"
#include "geo/int-rect.hh"
#include "geo/size.hh"

namespace faint{

int faint_cairo_stride(const IntSize&);

inline size_t multiply(size_t stride, size_t height){
  if (stride > std::numeric_limits<size_t>::max() / height){
    throw BitmapException("Bitmap length size_t overflow");
  }
  return stride * height;
}

inline size_t data_length(int stride, int height){
  if (height <= 0){
    throw BitmapException(height == 0 ?
      "Invalid height (0)" :
      "Negative height");
  }
  if (stride <= 0){
    if (stride == -1){
      // cairo_format_stride_for_width returns -1 if "the format is invalid
      // or the width too large".
      throw BitmapException("Invalid stride (-1)");
    }
    throw BitmapException(stride == 0 ? "Invalid stride (0)" : "Negative stride");
  }

  return multiply(to_size_t(stride), to_size_t(height));
}

inline uchar* allocate_bitmap_data(size_t len){
  try {
    return new uchar[len];
  }
  catch (const std::bad_alloc&){
    throw BitmapException("bad_alloc");
  }
}

Bitmap::Bitmap()
  : m_row_stride(0)
{
  m_w = m_h = 0;
  m_data = nullptr;
}

Bitmap::Bitmap(const Bitmap& other)
  : m_row_stride(other.m_row_stride),
    m_w(other.m_w),
    m_h(other.m_h),
    m_data(nullptr)
{
  if (other.m_data != nullptr){
    const auto len = data_length(m_row_stride, m_h);
    m_data = allocate_bitmap_data(len);
    memcpy(m_data, other.m_data, len);
  }
}

Bitmap::Bitmap(const IntSize& sz)
  : m_row_stride(0),
    m_w(sz.w),
    m_h(sz.h),
    m_data(nullptr)
{
  assert(m_w > 0);
  assert(m_h > 0);
  m_row_stride = faint_cairo_stride(sz);
  const auto len = data_length(m_row_stride, m_h);
  m_data = allocate_bitmap_data(len);
  memset(m_data, 0, len);
}

Bitmap::Bitmap(const IntSize& sz, const Color& bgColor)
  : m_row_stride(0),
    m_w(sz.w),
    m_h(sz.h),
    m_data(nullptr)
{
  assert(m_w > 0);
  assert(m_h > 0);
  m_row_stride = faint_cairo_stride(sz);
  const auto len = data_length(m_row_stride, m_h);
  m_data = allocate_bitmap_data(len);
  clear(*this, bgColor);
}

Bitmap::Bitmap(const IntSize& sz, const Paint& bg)
  : m_row_stride(0),
    m_w(sz.w),
    m_h(sz.h),
    m_data(nullptr)
{
  assert(m_w > 0);
  assert(m_h > 0);
  m_row_stride = faint_cairo_stride(sz);
  const auto len = data_length(m_row_stride, m_h);
  m_data = allocate_bitmap_data(len);
  clear(*this, bg);
}

Bitmap::Bitmap(const IntSize& sz, int stride)
  : m_row_stride(stride),
    m_w(sz.w),
    m_h(sz.h),
    m_data(nullptr)
{
  assert(sz.w > 0);
  assert(sz.h > 0);
  assert(m_row_stride >= sz.w);

  const auto len = data_length(m_row_stride, m_h);
  m_data = allocate_bitmap_data(len);
  memset(m_data, 0, len);
}

Bitmap::Bitmap(Bitmap&& source)
  : m_row_stride(source.m_row_stride),
    m_w(source.m_w),
    m_h(source.m_h),
    m_data(source.m_data)
{
  source.m_data = nullptr;
  source.m_w = 0;
  source.m_h = 0;
}

Bitmap::~Bitmap(){
  delete[] m_data;
}

Bitmap& Bitmap::operator=(const Bitmap& other){
  Bitmap copy(other);
  copy.Swap(*this);
  return *this;
}

void Bitmap::Swap(Bitmap& other){
  using std::swap;
  swap(m_row_stride, other.m_row_stride);
  swap(m_w, other.m_w);
  swap(m_h, other.m_h);
  swap(m_data, other.m_data);
}

Bitmap& Bitmap::operator=(Bitmap&& other){
  Bitmap copy(std::move(other));
  copy.Swap(*this);
  return *this;
}

bool Bitmap::operator==(const Bitmap& other) const{
  if (m_w != other.m_w || m_h != other.m_h){
    return false;
  }

  uchar* lhs = m_data;
  uchar* rhs = other.m_data;

  int lhsStride = m_row_stride;
  int rhsStride = other.m_row_stride;
  for (int y = 0; y != m_h; y++){
    for (int x = 0; x != m_w; x++){
      int iLhs = y * lhsStride + x * BPP;
      int iRhs = y * rhsStride + x * BPP;
      if (lhs[iLhs + iR] != rhs[iRhs + iR] ||
        lhs[iLhs + iG] != rhs[iRhs + iG] ||
        lhs[iLhs + iB] != rhs[iRhs + iB] ||
        lhs[iLhs + iA] != rhs[iRhs + iA]){
        return false;
      }
    }
  }
  return true;
}

bool Bitmap::operator!=(const Bitmap& other) const{
  return !operator==(other);
}

bool bitmap_ok(const Bitmap& bmp){
  return bmp.m_w != 0 && bmp.m_h != 0;
}

void clear(Bitmap& bmp, const Color& c){
  uchar* data = bmp.m_data;
  for (int y = 0; y != bmp.m_h; y++){
    for (int x = 0; x != bmp.m_w; x++){
      int dst = y * bmp.m_row_stride + x * BPP;
      data[dst + iR] = c.r;
      data[dst + iG] = c.g;
      data[dst + iB] = c.b;
      data[dst + iA] = c.a;
    }
  }
}

void clear(Bitmap& bmp, const ColRGB& c){
  clear(bmp, Color(c, 255));
}

void clear(Bitmap& bmp, const Paint& paint){
  fill_rect(bmp, IntRect(IntPoint(0,0), bmp.GetSize()), paint);
}

Color get_color(const Bitmap& bmp, const IntPoint& pos){
  return get_color_raw(bmp, pos.x, pos.y);
}

Color get_color_raw(const Bitmap& bmp, int x, int y){
  int pos = y * bmp.m_row_stride + x * BPP;
  uchar* data = bmp.m_data;
  return Color(data[pos + iR],
    data[pos + iG],
    data[pos + iB],
    data[pos + iA]);
}

bool fully_inside(const IntRect& r, const Bitmap& bmp){
  if (r.x < 0 || r.y < 0){
    return false;
  }
  if (r.x + r.w > bmp.m_w || r.y + r.h > bmp.m_h){
    return false;
  }
  return true;
}

bool is_blank(const Bitmap& bmp){
  Color color = get_color(bmp, IntPoint(0,0));
  for (int y = 0; y != bmp.m_h; y++){
    for (int x = 0; x != bmp.m_w; x++){
      if (get_color(bmp, IntPoint(x,y)) != color){
        return false;
      }
    }
  }
  return true;
}

bool point_in_bitmap(const Bitmap& bmp, const IntPoint& pos){
  return pos.x >= 0 && pos.y >= 0 && pos.x < bmp.m_w && pos.y < bmp.m_h;
}

void put_pixel(Bitmap& bmp, const IntPoint& pos, const Color& color){
  put_pixel_raw(bmp, pos.x, pos.y, color);
}

void put_pixel_raw(Bitmap& bmp, int x, int y, const Color& color){
  if (x < 0 || y < 0 || x >= bmp.m_w || y >= bmp.m_h) {
    return;
  }
  uchar* data = bmp.GetRaw();
  int pos = y * bmp.m_row_stride + x * BPP;
  data[pos + iA] = color.a;
  data[pos + iR] = color.r;
  data[pos + iG] = color.g;
  data[pos + iB] = color.b;
}

// A dumb hard-coded "recommended" max area. Allocating larger bitmaps
// is likely to fail (at least when built for 32-bit)
constexpr int BITMAP_MAX_AREA = 13000 * 13000;

bool reasonable_bitmap_size(const IntSize& size){
  return size.w > 0 && size.h > 0 &&
    area_less(size, BITMAP_MAX_AREA);
}

bool reasonable_bitmap_size(const Size& size){
  return size.w > 0 && size.h > 0 &&
    area_less(size, BITMAP_MAX_AREA);
}

} // namespace faint
