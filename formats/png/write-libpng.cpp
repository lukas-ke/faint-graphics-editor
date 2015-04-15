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

#include <png.h> // libpng
#include "formats/png/png-util.hh"
#include "formats/png/write-libpng.hh"
#include "geo/limits.hh"

#ifdef _MSC_VER
#pragma warning(disable:4611) // _setjmp and C++-object destruction
#endif

namespace faint{

static png_byte gray_sum(int r, int g, int b){
  return static_cast<png_byte>((r + g + b) / 3);
}

static void free_rows(png_bytep* rowPointers, png_uint_32 height){
  for (png_uint_32 y = 0; y != height; y++){
    free(rowPointers[y]);
  }
  free(rowPointers);
}

PngWriteResult write_with_libpng(const char* path,
  const Bitmap& bmp,
  const int colorType,
  const png_tEXt_map& textChunks)
{
  assert(bitmap_ok(bmp));
  assert(rgb_or_rgba(colorType) || gray_or_gray_alpha(colorType));

  png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
    nullptr, nullptr, nullptr);

  if (png_ptr == nullptr){
    return PngWriteResult::ERROR_CREATE_WRITE_STRUCT;
  }

  FILE* f = fopen(path, "wb");
  if (!f){
    return PngWriteResult::ERROR_OPEN_FILE;
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == nullptr){
    fclose(f);
    return PngWriteResult::ERROR_CREATE_INFO_STRUCT;
  }

  if (setjmp(png_jmpbuf(png_ptr))){
    fclose(f);
    return PngWriteResult::ERROR_INIT_IO;
  }

  png_init_io(png_ptr, f);

  // Write the PNG-header
  if (setjmp(png_jmpbuf(png_ptr))){
    fclose(f);
    return PngWriteResult::ERROR_WRITE_HEADER;
  }

  auto size(bmp.GetSize());
  assert(size.w != 0 && size.h != 0);
  const png_uint_32 width = convert(size.w);
  const png_uint_32 height = convert(size.h);
  const png_byte bitDepth = 8;

  png_set_IHDR(png_ptr,
    info_ptr,
    width,
    height,
    bitDepth,
    colorType,
    PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_BASE,
    PNG_FILTER_TYPE_BASE);

  if (textChunks.size() != 0){

    if (!can_represent<int>(textChunks.size())){
      fclose(f);
      return PngWriteResult::ERROR_WRITE_TOO_MANY_TEXT_CHUNKS;
    }

    const int numChunks = static_cast<int>(textChunks.size());
    png_text* textItems = new png_text[numChunks];
    size_t i = 0;

    for (const auto& kv : textChunks){
      const auto& key = kv.first;
      if (!is_ascii(key)){
        delete[] textItems;
        fclose(f);
        return PngWriteResult::ERROR_WRITE_TEXT_KEY_ENCODING;
      }

      if (key.size() > PNG_KEYWORD_MAX_LENGTH){
        delete[] textItems;
        fclose(f);
        return PngWriteResult::ERROR_WRITE_TEXT_KEY_TOO_LONG;
      }
      else if (key.size() == 0){
        delete[] textItems;
        fclose(f);
        return PngWriteResult::ERROR_WRITE_TEXT_KEY_EMPTY;
      }

      const auto& value = kv.second;
      if (!is_ascii(value)){
        delete[] textItems;
        fclose(f);
        return PngWriteResult::ERROR_WRITE_TEXT_VALUE_ENCODING;
      }

      if (!can_represent<png_size_t>(value.size())){
        delete[] textItems;
        fclose(f);
        return PngWriteResult::ERROR_WRITE_TEXT_VALUE_TOO_LONG;
      }

      auto& text(textItems[i]);
      text.compression = -1; // tEXt
      text.key = (png_charp)key.c_str(); // Fixme
      text.text = (png_charp)value.c_str(); // Fixme
      text.text_length = value.size();
      i++;
    }

    png_set_text(png_ptr, info_ptr, textItems, numChunks);
    delete[] textItems; // Fixme: Safe or wait?
  }

  png_write_info(png_ptr, info_ptr);

  // Create raw-data arrays for libpng from the Bitmap.
  png_byte** rowPointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
  const auto bytesPerRow = png_get_rowbytes(png_ptr, info_ptr);

  if (rgb_or_rgba(colorType)){
    // Write RGB or RGBA

    const int PNG_BPP = colorType == PNG_COLOR_TYPE_RGBA ? 4 : 3;
    assert((width - 1) * PNG_BPP + (PNG_BPP - 1) < bytesPerRow);

    const auto* src = bmp.GetRaw();
    const png_uint_32 stride = convert(bmp.GetStride());
    const png_uint_32 srcBpp = convert(faint::BPP);
    for (png_uint_32 y = 0; y != height; y++){
      auto row = (png_byte*) malloc(bytesPerRow);
      rowPointers[y] = row;
      for (png_uint_32 x = 0; x != width; x++){
        // Fill the row with the bitmap-data
        auto i = y * stride + x * srcBpp;
        row[x * PNG_BPP] = src[i + faint::iR];
        row[x * PNG_BPP + 1] = src[i + faint::iG];
        row[x * PNG_BPP + 2] = src[i + faint::iB];

        if (PNG_BPP == 4){
          row[x * PNG_BPP + 3] = src[i + faint::iA];
        }
      }
    }
  }

  else if (gray_or_gray_alpha(colorType)){
    // Write gray-scale

    const int PNG_BPP = colorType == PNG_COLOR_TYPE_GRAY_ALPHA ? 2 : 1;
    assert((width - 1) * PNG_BPP + (PNG_BPP - 1) < bytesPerRow);

    const auto* src = bmp.GetRaw();
    const png_uint_32 stride = convert(bmp.GetStride());
    const png_uint_32 srcBpp = convert(faint::BPP);

    for (png_uint_32 y = 0; y != height; y++){
      auto row = (png_byte*) malloc(bytesPerRow);
      rowPointers[y] = row;

      for (png_uint_32 x = 0; x != width; x++){
        // Fill the row with the bitmap-data
        const auto i = y * stride + x * srcBpp;

        row[x * PNG_BPP] = gray_sum(src[i + faint::iR],
          src[i + faint::iG],
          src[i + faint::iB]);
        if (PNG_BPP == 2){
          row[x * PNG_BPP + 1] = src[i + faint::iA];
        }
      }
    }
  }

  // Write the image data
  if (setjmp(png_jmpbuf(png_ptr))){
    free_rows(rowPointers, height);
    fclose(f);
    return PngWriteResult::ERROR_WRITE_DATA;
  }
  png_write_image(png_ptr, rowPointers);

  // Write end
  if (setjmp(png_jmpbuf(png_ptr))){
    free_rows(rowPointers, height);
    fclose(f);
    return PngWriteResult::ERROR_WRITE_END;
  }
  png_write_end(png_ptr, nullptr);

  free_rows(rowPointers, height);
  fclose(f);
  return PngWriteResult::OK;
}

} // namespace
