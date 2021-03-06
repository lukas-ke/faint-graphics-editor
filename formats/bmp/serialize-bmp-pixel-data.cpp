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

#include "bitmap/alpha-map.hh"
#include "bitmap/bitmap.hh"
#include "formats/bmp/bmp-types.hh" // For BmpSizeAndOrder
#include "bitmap/color-list.hh"
#include "bitmap/quantize.hh"
#include "formats/bmp/serialize-bmp-pixel-data.hh"
#include "util-wx/stream.hh"

namespace faint{

int bmp_row_stride(int bitsPerPixel, int w){
  // Round the row size to a multiple of four bytes.
  const int ROW_PAD_BYTES = 4;
  return ((bitsPerPixel * w + 31) / 32) * ROW_PAD_BYTES;
}

int and_map_row_stride(int w){
  return ((w % 32) == 0) ? w / 8 :
    4 * (w / 32 + 1);
}

int and_map_bytes(const IntSize& bmpSize){
  return and_map_row_stride(bmpSize.w) * bmpSize.h;
}

static void write_color_table(BinaryWriter& out, const ColorList& l){
  assert(l.size() <= 256);
  for (const auto& c : l){
    out.put(c.b);
    out.put(c.g);
    out.put(c.r);
    out.put(0);
  }
}

Optional<AlphaMap> read_1bipp_BI_RGB(BinaryReader& in,
  const BmpSizeAndOrder& bmpSize)
{
  const auto& size = bmpSize.size;
  const int rowLength = bmp_row_stride<1>(size.w);
  const int bufferLength = rowLength * size.h;

  std::vector<char> pixelData(bufferLength);
  in.read(pixelData.data(), bufferLength);
  if (!in.good()){
    return {};
  }

  AlphaMap alphaMap(size);
  for (int y = 0; y != size.h; y++){
    for (int x = 0; x != size.w; x++){
      unsigned int value =
        static_cast<unsigned int>(pixelData[rowLength * y + x / 8]);
      unsigned char paletteIndex = (value & (1 << (7 - x % 8))) == 0 ? 0 : 1;
      const auto yDst = bmpSize.topDown ? y : size.h - y - 1;
      alphaMap.Set(x, yDst, paletteIndex);
    }
  }
  return option(alphaMap);
}

Optional<AlphaMap> read_4bipp_BI_RGB(BinaryReader& in,
  const BmpSizeAndOrder& bmpSize)
{
  const auto& size = bmpSize.size;
  const int rowLength = bmp_row_stride<4>(size.w);
  const int bufferLength = rowLength * size.h;

  std::vector<char> pixelData(bufferLength);
  in.read(pixelData.data(), bufferLength);
  if (!in.good()){
    return {};
  }

  AlphaMap alphaMap(size);
  for (int y = 0; y != size.h; y++){
    for (int x = 0; x != size.w; x++){
      unsigned int value =
        static_cast<unsigned int>(pixelData[rowLength * y + x / 2]);
      if (x % 2 != 0){
        value <<= 4;
      }
      value &= 0xf;

      const auto yDst = bmpSize.topDown ? y : size.h - y - 1;
      alphaMap.Set(x, yDst, static_cast<uchar>(value));
    }
  }
  return option(alphaMap);
}

Optional<AlphaMap> read_8bipp_BI_RGB(BinaryReader& in,
  const BmpSizeAndOrder& bmpSize)
{
  const auto& size = bmpSize.size;
  const int padBytes = bmp_row_padding<8>(size.w);
  AlphaMap alphaMap(size);

  for (int y = 0; y != size.h; y++){
    for (int x = 0; x != size.w; x++){
      // Fixme: Read full rows
      char v;
      in.read(&v, 1);
      if (!in.good()){
        return option(alphaMap); // Fixme
        // return {};
      }

      const auto yDst = bmpSize.topDown ? y : size.h - y - 1;
      alphaMap.Set(x, yDst, static_cast<unsigned char>(v));
    }
    in.ignore(padBytes);
  }
  return option(alphaMap);
}

Optional<Bitmap> read_24bipp_BI_RGB(BinaryReader& in,
  const BmpSizeAndOrder& bmpSize)
{
  const auto& size = bmpSize.size;
  Bitmap bmp(size);
  const int padBytes = bmp_row_padding<24>(size.w);
  for (int y = 0; y != bmp.m_h; y++){
    for (int x = 0; x != bmp.m_w; x++){
      // Fixme: Read full rows, or even whole image.
      char bytes[3];
      in.read(bytes, 3);
      if (!in.good()){
        return {};
      }
      const auto yDst = bmpSize.topDown ? y : size.h - y - 1;
      put_pixel_raw(bmp, x, yDst,
        Color(static_cast<unsigned char>(bytes[2]),
          static_cast<unsigned char>(bytes[1]),
          static_cast<unsigned char>(bytes[0]),
          255));
    }
    in.ignore(padBytes);
  }
  return option(bmp);
}

Optional<Bitmap> read_32bipp_BI_RGB(BinaryReader& in,
  const BmpSizeAndOrder& bmpSize)
{
  const auto& size(bmpSize.size);
  Bitmap bmp(size);
  const int padBytes = bmp_row_padding<32>(size.w);

  for (int y = 0; y != bmp.m_h; y++){
    for (int x = 0; x != bmp.m_w; x++){
      char bytes[4];
      in.read(bytes, 4);
      if (!in.good()){
        return {};
      }
      const auto yDst = bmpSize.topDown ? y : size.h - y - 1;
      put_pixel_raw(bmp, x, yDst,
        Color(static_cast<unsigned char>(bytes[3]),
          static_cast<unsigned char>(bytes[2]),
          static_cast<unsigned char>(bytes[1]),
          static_cast<unsigned char>(bytes[0])));
    }
    in.ignore(padBytes);
  }
  return option(bmp);
}

Optional<ColorList> read_color_table(BinaryReader& in, int numColors){
  // blue, green, red, 0x00
  ColorList l;
  l.reserve(numColors);
  for (int i = 0; i != numColors; i++){
    char bytes[4];
    in.read(bytes, 4);
    if (!in.good()){
      return {};
    }
    l.push_back(ColRGB(static_cast<unsigned char>(bytes[2]),
        static_cast<unsigned char>(bytes[1]),
        static_cast<unsigned char>(bytes[0])));
  }
  return option(l);
}

void write_8bipp_BI_RGB(BinaryWriter& out, const MappedColors& mappedColors){
  const auto& map(mappedColors.map);
  write_color_table(out, mappedColors.palette);

  const IntSize sz(map.GetSize());
  int padBytes = bmp_row_padding<8>(sz.w);
  for (int y = 0; y != sz.h; y++){
    for (int x = 0; x != sz.w; x++){
      out.put(map.Get(x, sz.h - y - 1));
    }
    for (int i = 0; i != padBytes; i++){
      out.put(0);
    }
  }
}

void write_24bipp_BI_RGB(BinaryWriter& out, const Bitmap& bmp){
  const int padBytes = bmp_row_padding<24>(bmp.m_w);

  for (int y = 0; y != bmp.m_h; y++){
    for (int x = 0; x != bmp.m_w; x++){
      Color c(get_color_raw(bmp, x, bmp.m_h - y - 1));
      out.put(c.b);
      out.put(c.g);
      out.put(c.r);
    }
    for (int i = 0; i != padBytes; i++){
      out.put(0);
    }
  }
}

void write_32bipp_BI_RGB_ICO(BinaryWriter& out, const Bitmap& bmp){
  // The size from the bmp-header. May have larger height than the size
  // in the IconDirEntry. (Fixme: Why?)

  // Write the pixel data (in archaic ICO-terms, the XOR-mask)
  for (int y = 0; y != bmp.m_h; y++){
    for (int x = 0; x != bmp.m_w; x++){
      Color c(get_color_raw(bmp, x, bmp.m_h - y - 1));
      out.put(c.b);
      out.put(c.g);
      out.put(c.r);
      out.put(c.a);
    }
  }

  // Write the AND-mask
  int len = and_map_bytes(bmp.GetSize());
  for (int i = 0; i != len; i++){
    out.put(static_cast<char>(0xffu));
    // <../../doc/fuuuu.png>
  }
}

} // namespace
