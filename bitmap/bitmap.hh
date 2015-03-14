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

#ifndef FAINT_BITMAP_HH
#define FAINT_BITMAP_HH
#include "bitmap/paint-fwd.hh"
#include "geo/geo-fwd.hh"
#include "geo/int-size.hh"
#include "geo/primitive.hh"

namespace faint{

const int BPP = 4;
const int iR = 2;
const int iG = 1;
const int iB = 0;
const int iA = 3;

class Bitmap {
  // ARGB32 Bitmap
public:
  // Initializes an invalid Bitmap. Must be assigned to before use
  Bitmap();
  Bitmap(const Bitmap&);
  Bitmap(const IntSize&);
  Bitmap(const IntSize&, const Color&);
  Bitmap(const IntSize&, const Paint&);
  Bitmap(const IntSize&, int stride);
  Bitmap(Bitmap&&);
  ~Bitmap();
  inline uchar* GetRaw(){
    return m_data;
  }

  inline const uchar* GetRaw() const{
    return m_data;
  }

  IntSize GetSize() const{
    return IntSize(m_w, m_h);
  }

  inline int GetStride() const{
    return m_row_stride;
  }

  void Swap(Bitmap&);

  Bitmap& operator=(const Bitmap&);
  Bitmap& operator=(Bitmap&&);

  bool operator==(const Bitmap&) const;
  bool operator!=(const Bitmap&) const;

  int m_row_stride;
  int m_w;
  int m_h;
  uchar* m_data;
};

bool bitmap_ok(const Bitmap&);
void clear(Bitmap&, const Color&);
void clear(Bitmap&, const ColRGB&);
void clear(Bitmap&, const Paint&);
bool fully_inside(const IntRect&, const Bitmap&);
Color get_color(const Bitmap&, const IntPoint&);
Color get_color_raw(const Bitmap&, int x, int y);
bool is_blank(const Bitmap&);
bool point_in_bitmap(const Bitmap&, const IntPoint&);
void put_pixel(Bitmap&, const IntPoint&, const Color&);
void put_pixel_raw(Bitmap&, int x, int y, const Color&);

// True if the size isn't very large, to avoid attempting to
// allocate bitmaps of sizes likely to fail (for a 32-bit compile)
bool reasonable_bitmap_size(const IntSize&);

class Size;
bool reasonable_bitmap_size(const Size&);

class DstBmp{
  // For indicating a target bitmap in Bitmap operations,
  // Should be created using the onto(Bitmap&)-function
public:
  explicit DstBmp(Bitmap& bmp) :
    m_bmp(bmp)
  {}

  Bitmap& Get() const{
    return m_bmp;
  }

  uchar* GetRaw() const{
    return m_bmp.m_data;
  }

  IntSize GetSize() const{
    return m_bmp.GetSize();
  }

  int GetStride() const{
    return m_bmp.m_row_stride;
  }

  DstBmp& operator=(const DstBmp&) = delete;
private:
  Bitmap& m_bmp;
};

inline DstBmp onto(Bitmap& bmp){
  return DstBmp(bmp);
}

} // namespace faint

#endif
