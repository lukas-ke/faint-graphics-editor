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
#include "bitmap/color-list.hh"
#include "bitmap/quantize.hh"
#include "formats/bmp/bmp-errors.hh"
#include "formats/bmp/serialize-bmp-types.hh"
#include "formats/bmp/file-bmp.hh"
#include "formats/bmp/serialize-bmp-pixel-data.hh"
#include "util/serialize-tuple.hh"
#include "util-wx/stream.hh"

namespace faint{

static int palette_length_bytes(BitmapQuality quality){
  switch (quality){
  case BitmapQuality::COLOR_24BIT:
    // No palette, RGBA stored directly
    return 0;
  case BitmapQuality::GRAY_8BIT:
  case BitmapQuality::COLOR_8BIT:
    // 8-bit-per-pixel color table.
    return 4 * 256;
  }
  assert(false);
  return 0;
}

static BitmapFileHeader create_bitmap_file_header(BitmapQuality quality,
  int rowStride,
  int height)
{
  const auto headerLengths = struct_lengths<BitmapInfoHeader, BitmapFileHeader>();

  BitmapFileHeader h;
  h.fileType = 0x4d42; // "BM" (reversed, endianness and all)

  h.fileLength = static_cast<uint32_t>(headerLengths + 
    palette_length_bytes(quality) + 
    rowStride * height); // Fixme: Check cast

  h.reserved1 = 0;
  h.reserved2 = 0;

  h.dataOffset = static_cast<uint32_t>(headerLengths + 
    palette_length_bytes(quality)); // Fixme: Check cast
  return h;
}

static ColorList grayscale_color_table(){
  ColorList l;
  for (int i = 0; i != 256; i++){
    l.AddColor({static_cast<uchar>(i),
      static_cast<uchar>(i),
      static_cast<uchar>(i),
      255});
  }
  return l;
}

static Bitmap read_bmp_or_throw(const FilePath& filePath){
  BinaryReader in(filePath);
  if (!in.good()){
    throw ReadBmpError(error_open_file_read(filePath));
  }

  // Fixme: Handle eof
  auto bitmapFileHeader = read_struct_or_throw<BitmapFileHeader>(in);

  // Fixme: Again reversed. Split into bytes
  if (bitmapFileHeader.fileType != 0x4d42){
    throw ReadBmpError(error_bitmap_signature(bitmapFileHeader.fileType));
  }

  auto bitmapInfoHeader = read_struct_or_throw<BitmapInfoHeader>(in);

  if (invalid_header_length(bitmapInfoHeader.headerLen)){
    // Fixme: Won't happen, might be greater than 40 though.
    throw ReadBmpError(error_truncated_bmp_header(0, bitmapInfoHeader.headerLen));
  }

  if (bitmapInfoHeader.compression != Compression::BI_RGB){
    throw ReadBmpError(error_compression(0, bitmapInfoHeader.compression));
  }

  if (bitmapInfoHeader.colorPlanes != 1){
    throw ReadBmpError(error_color_planes(0, bitmapInfoHeader.colorPlanes));
  }

  const IntSize bmpSize(bitmapInfoHeader.width, bitmapInfoHeader.height);

  if (bitmapInfoHeader.bpp == 8){
    auto colorList = or_throw(read_color_table(in, 256),
      "Failed reading color table.");

    in.seekg(bitmapFileHeader.dataOffset);

    auto alphaMap = or_throw(read_8bpp_BI_RGB(in, bmpSize),
      "Failed reading 8bpp-pixel data");

    return bitmap_from_indexed_colors(alphaMap, colorList);
  }
  else {
    in.seekg(bitmapFileHeader.dataOffset);
    if (bitmapInfoHeader.bpp == 24){
      return or_throw(read_24bpp_BI_RGB(in, bmpSize),
        "Failed reading 24-bpp pixel data.");
    }
    else if (bitmapInfoHeader.bpp == 32){
      return or_throw(read_32bpp_BI_RGB(in, bmpSize),
        "Failed reading 32-bpp pixel data.");
    }
  }
  throw ReadBmpError("Unsupported bpp");
}

OrError<Bitmap> read_bmp(const FilePath& filePath){
  try{
    return read_bmp_or_throw(filePath);
  }
  catch (const ReadBmpError& error){
    return error.message;
  }
}

// Fixme: Pass a channel_t instead, and quantize outside
static AlphaMap desaturate_AlphaMap(const Bitmap& bmp){
  AlphaMap a(bmp.GetSize());
  for (int y = 0; y != bmp.m_h; y++){
    for (int x = 0; x != bmp.m_w; x++){
      Color c(get_color_raw(bmp, x, y));
      a.Set(x, y, static_cast<uchar>(sum_rgb(c) / 3));
    }
  }
  return a;
}

SaveResult write_bmp(const FilePath& filePath,
  const Bitmap& bmp,
  BitmapQuality quality)
{
  BinaryWriter out(filePath);
  if (!out.good()){
    return SaveResult::SaveFailed(error_open_file_write(filePath));
  }

  uint16_t bpp = quality == BitmapQuality::COLOR_24BIT ? 24 : 8;

  const IntSize size(bmp.GetSize());
  const int rowStride = bmp_row_stride(bpp, size.w);

  write_struct(out, create_bitmap_file_header(quality, rowStride, size.h));
  write_struct(out, create_bitmap_info_header(bmp, bpp, false));

  switch(quality){
    // Note: No default, to ensure warning if unhandled enum value
  case BitmapQuality::COLOR_8BIT:
    write_8bpp_BI_RGB(out, quantized(bmp, Dithering::ON));
    return SaveResult::SaveSuccessful();

  case BitmapQuality::GRAY_8BIT:
    write_8bpp_BI_RGB(out, {desaturate_AlphaMap(bmp), grayscale_color_table()});
    return SaveResult::SaveSuccessful();

  case BitmapQuality::COLOR_24BIT:
    write_24bpp_BI_RGB(out, bmp); // Fixme
    return SaveResult::SaveSuccessful();
  }

  assert(false);
  return SaveResult::SaveFailed(utf8_string("Internal error in save_bitmap"));
}

} // namespace
