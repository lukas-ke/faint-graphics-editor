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

#include "formats/faint-fopen.hh"
#include "formats/png/read-libpng.hh"

#ifdef _MSC_VER
#pragma warning(disable:4611) // _setjmp and C++-object destruction
#endif

namespace faint{

PngReadResult read_with_libpng(const FilePath& path,
  png_byte** rows,
  png_uint_32* width,
  png_uint_32* height,
  png_byte* colorType,
  png_byte* bitDepth,
  int* bitsPerPixel,
  png_color** palette,
  int* numPalette,
  std::map<utf8_string, utf8_string>& textChunks)
{
  FILE* f = faint_fopen_read_binary(path);
  if (f == nullptr){
    return PngReadResult::ERROR_OPEN_FILE;
  }

  png_byte sig[8];
  size_t readBytes = fread(sig, 1, 8, f);
  if (readBytes != 8){
    fclose(f);
    return PngReadResult::ERROR_PNG_SIGNATURE;
  }

  if (!png_check_sig(sig, 8)){
    fclose(f);
    return PngReadResult::ERROR_PNG_SIGNATURE;
  }

  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
    nullptr, nullptr, nullptr);

  if (png_ptr == nullptr){
    fclose(f);
    return PngReadResult::ERROR_CREATE_READ_STRUCT;
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == nullptr){
    png_destroy_read_struct(&png_ptr, nullptr, nullptr);
    fclose(f);
    return PngReadResult::ERROR_CREATE_INFO_STRUCT;
  }

  if (setjmp(png_jmpbuf(png_ptr))){
    // Fixme: Pass end_info
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    fclose(f);
    return PngReadResult::ERROR_INIT_IO;
  }

  png_init_io(png_ptr, f);
  png_set_sig_bytes(png_ptr, 8);
  png_read_info(png_ptr, info_ptr);

  png_textp textPtr;
  int numText;
  if (png_get_text(png_ptr, info_ptr, &textPtr, &numText) > 0){
    for (int i = 0; i != numText; i++){
      const auto& text(textPtr[i]);
      auto len = strlen(text.key);
      if (len == 0 || len > PNG_KEYWORD_MAX_LENGTH){
        continue; // Fixme: Signal error
      }
      if (strlen(text.text) != text.text_length){
        continue; // Fixme: Signal error
      }
      utf8_string key(text.key);
      utf8_string value(text.text);

      textChunks[key] = value;
    }
  }

  *width = png_get_image_width(png_ptr, info_ptr);
  *height = png_get_image_height(png_ptr, info_ptr);

  *colorType = png_get_color_type(png_ptr, info_ptr);
  *bitDepth = png_get_bit_depth(png_ptr, info_ptr);
  *bitsPerPixel = *bitDepth * png_get_channels(png_ptr, info_ptr);

  png_set_interlace_handling(png_ptr); // Fixme: returned num_passes irrelevant?
  png_read_update_info(png_ptr, info_ptr);

  if (setjmp(png_jmpbuf(png_ptr))){
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    fclose(f);
    return PngReadResult::ERROR_READ_DATA;
  }

  const auto rowBytes = png_get_rowbytes(png_ptr, info_ptr);
  if (rowBytes == 0){
    return PngReadResult::ERROR_READ_DATA;
  }

  *rows = (png_byte*) malloc(rowBytes * *height);
  if (*rows == nullptr){
    return PngReadResult::ERROR_MALLOC;
  }
  auto rowPointers = (png_byte**) malloc(sizeof(png_byte*) * (*height));
  if (rowPointers == nullptr){
    free(*rows);
    return PngReadResult::ERROR_MALLOC;
  }

  for (png_uint_32 y = 0; y < (*height); y++){
    rowPointers[y] = *rows + y * rowBytes;
  }

  png_read_image(png_ptr, rowPointers);
  // Fixme: Read post-image-data
  if (*colorType == PNG_COLOR_TYPE_PALETTE){
    png_color* tempPalette;
    auto result = png_get_PLTE(png_ptr, info_ptr, &tempPalette, numPalette);
    if (result != PNG_INFO_PLTE){
      return PngReadResult::ERROR_READ_PALETTE;
    }
    *palette = (png_color*) malloc(sizeof(png_color) * PNG_MAX_PALETTE_LENGTH);
    if (*palette == nullptr){
      free(*rows);
      free(*rowPointers);
      return PngReadResult::ERROR_MALLOC;
    }
    memcpy(*palette, tempPalette, *numPalette);
  }

  png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
  fclose(f);
  free(rowPointers);
  return PngReadResult::OK;
}

} // namespace
