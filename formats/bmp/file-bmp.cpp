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
#include "formats/bmp/file-bmp.hh"
#include "formats/bmp/serialize-bmp-pixel-data.hh"
#include "formats/bmp/serialize-bmp-types.hh"
#include "formats/format-errors-common.hh"
#include "geo/limits.hh"
#include "text/formatting.hh"
#include "util-wx/stream.hh"
#include "util/make-vector.hh"
#include "util/range-iter.hh"
#include "util/serialize-tuple.hh"

namespace faint{

static int palette_length_bytes(const PaletteColors& numColors){
  return static_cast<int>(numColors.Get() * 4);
}

static BitmapFileHeader create_bitmap_file_header(const PaletteColors& numColors,
  int rowStride,
  int height)
{
  const auto headerLengths = struct_lengths<BitmapInfoHeader, BitmapFileHeader>();

  BitmapFileHeader h;
  h.fileType = BITMAP_SIGNATURE;

  h.fileLength = convert(headerLengths +
    palette_length_bytes(numColors) +
    asserting_static_cast<uint32_t>(rowStride) *
    asserting_static_cast<uint32_t>(height));

  h.reserved1 = 0;
  h.reserved2 = 0;

  h.dataOffset = asserting_static_cast<uint32_t>(headerLengths +
    palette_length_bytes(numColors));
  return h;
}

static ColorList grayscale_color_table(){
  return make_vector(make_closed_range(0, 255), grayscale_rgb);
}

static Bitmap read_bmp_or_throw(const FilePath& filePath){
  BinaryReader in(filePath);
  if (!in.good()){
    throw ReadBmpError(error_open_file_read(filePath));
  }

  auto bitmapFileHeader = read_struct_or_throw_bmp<BitmapFileHeader>(in);

  if (bitmapFileHeader.fileType != BITMAP_SIGNATURE){
    throw ReadBmpError(error_bitmap_signature(bitmapFileHeader.fileType));
  }

  auto bitmapInfoHeader = read_struct_or_throw_bmp<BitmapInfoHeader>(in);

  if (invalid_header_length(bitmapInfoHeader.headerLen)){
    throw ReadBmpError(error_truncated_bmp_header(bitmapInfoHeader.headerLen));
  }

  if (bitmapInfoHeader.compression != Compression::BI_RGB){
    throw ReadBmpError(error_compression(bitmapInfoHeader.compression));
  }

  if (bitmapInfoHeader.colorPlanes != 1){
    throw ReadBmpError(error_color_planes(bitmapInfoHeader.colorPlanes));
  }

  auto bmpSize = get_size_and_order(bitmapInfoHeader);

  if (bitmapInfoHeader.paletteColors != 0){
    if (bitmapInfoHeader.bitsPerPixel != 8){ // Fixme: Maybe support?
      throw ReadBmpError("Palette for non 8-bpp bitmaps unsupported by Faint.");
    }
    auto colorList = or_throw(read_color_table(in, bitmapInfoHeader.paletteColors),
      "Failed reading color table.");
    in.seekg(bitmapFileHeader.dataOffset);

    auto alphaMap = or_throw(read_8bipp_BI_RGB(in, bmpSize),
      "Failed reading 8-bits-per-pixel data");

    return bitmap_from_indexed_colors(alphaMap, colorList);
  }
  else{
    // No palette
    in.seekg(bitmapFileHeader.dataOffset);
    if (bitmapInfoHeader.bitsPerPixel == 8){
      auto alphaMap = or_throw(read_8bipp_BI_RGB(in, bmpSize),
        "Failed reading 8-bits-per-pixel data");
      return bitmap_from_indexed_colors(alphaMap, grayscale_color_table());
    }
    else if (bitmapInfoHeader.bitsPerPixel == 24){
      return or_throw(read_24bipp_BI_RGB(in, bmpSize),
        "Failed reading 24-bits-per-pixel data.");
    }
    else if (bitmapInfoHeader.bitsPerPixel == 32){
      return or_throw(read_32bipp_BI_RGB(in, bmpSize),
        "Failed reading 32-bits-per-pixel data.");
    }
  }
  throw ReadBmpError(Sentence("Unsupported bits-per-pixel",
    bracketed(str_int(bitmapInfoHeader.bitsPerPixel))));
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

static uint16_t bits_per_pixel(BitmapQuality quality){
  switch(quality){
  case BitmapQuality::COLOR_8BIT:
    return 8;
  case BitmapQuality::GRAY_8BIT:
    return 8;
  case BitmapQuality::COLOR_24BIT:
    return 24;
  }
  assert(false);
  return 0;
}

SaveResult write_bmp(const FilePath& filePath,
  const Bitmap& bmp,
  BitmapQuality quality)
{
  BinaryWriter out(filePath);
  if (!out.good()){
    return SaveResult::SaveFailed(error_open_file_write(filePath));
  }

  const auto bitsPerPixel = bits_per_pixel(quality);
  const IntSize size(bmp.GetSize());
  const int rowStride = bmp_row_stride(bitsPerPixel, size.w);

  switch(quality){
    // Note: No default, to ensure warning if unhandled enum value
  case BitmapQuality::COLOR_8BIT:
    {
      const auto pixelData = quantized(bmp, Dithering::ON);
      PaletteColors paletteColors(pixelData.palette.size());

      write_struct(out,
        create_bitmap_file_header(paletteColors, rowStride, size.h));
      write_struct(out,
        create_bitmap_info_header_8bipp(bmp.GetSize(),
          default_DPI(),
          PaletteColors(pixelData.palette.size()),
          false));
      write_8bipp_BI_RGB(out, pixelData);
      return SaveResult::SaveSuccessful();
    }

  case BitmapQuality::GRAY_8BIT:
    {
      MappedColors pixelData(desaturate_AlphaMap(bmp), grayscale_color_table());
      PaletteColors paletteColors(pixelData.palette.size());
      write_struct(out,
        create_bitmap_file_header(paletteColors, rowStride, size.h));
      write_struct(out,
        create_bitmap_info_header_8bipp(bmp.GetSize(),
          default_DPI(),
          PaletteColors(pixelData.palette.size()),
          false));
      write_8bipp_BI_RGB(out, pixelData);
      return SaveResult::SaveSuccessful();
    }

  case BitmapQuality::COLOR_24BIT:
    {
      write_struct(out,
        create_bitmap_file_header(PaletteColors(0), rowStride, size.h));
      write_struct(out,
        create_bitmap_info_header_24bipp(bmp.GetSize(),
          default_DPI(),
          false));
      write_24bipp_BI_RGB(out, bmp);
      return SaveResult::SaveSuccessful();
    }
  }

  assert(false);
  return SaveResult::SaveFailed(utf8_string("Internal error in save_bitmap"));
}

} // namespace
