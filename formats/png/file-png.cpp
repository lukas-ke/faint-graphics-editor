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

#include "png.h" // libpng

#include "bitmap/bitmap-exception.hh"
#include "formats/png/file-png.hh"
#include "geo/limits.hh"
#include "text/utf8-string.hh"

#ifdef _MSC_VER
#pragma warning(disable:4611) // _setjmp and C++-object destruction
#endif

namespace faint{

enum class PngResult{
  OK,
  ERROR_OPEN_FILE,
  ERROR_PNG_SIGNATURE,
  ERROR_CREATE_READ_STRUCT,
  ERROR_CREATE_INFO_STRUCT,
  ERROR_INIT_IO,
  ERROR_READ_DATA
};

static utf8_string to_string(PngResult result){
  // Fixme: Compare error text style with bmp-load errors etc
  using R = PngResult;
  if (result == R::OK){
    return "No error";
  }
  else if (result == R::ERROR_OPEN_FILE){
    return "Failed opening file for reading.";
  }
  else if (result == R::ERROR_PNG_SIGNATURE){
    return "Incorrect PNG signature.";
  }
  else if (result == R::ERROR_CREATE_READ_STRUCT){
    return "Failed creating read structure"; // Fixme: Memory? Png init?
  }
  else if (result == R::ERROR_CREATE_INFO_STRUCT){
    return "Failed creating info structure"; // Fixme: Memory? Png init?
  }
  else if (result == R::ERROR_INIT_IO){
    return "Failed creating initializing IO"; // Fixme: Memory? Png init?
  }
  else if (result == R::ERROR_READ_DATA){
    return "Failed reading image data.";
  }
  else{
    return "Unknown error.";
  }
}

PngResult read_with_libpng(const char* path,
  png_bytep** rowPointers,
  png_uint_32* width,
  png_uint_32* height,
  png_byte* colorType,
  png_byte* bitDepth)
{
  FILE* f = fopen(path, "rb");
  if (f == nullptr){
    return PngResult::ERROR_OPEN_FILE;
  }

  png_byte sig[8];
  size_t readBytes = fread(sig, 1, 8, f);
  if (readBytes != 8){
    return PngResult::ERROR_PNG_SIGNATURE;
  }

  if (!png_check_sig(sig, 8)){
    return PngResult::ERROR_PNG_SIGNATURE;
  }

  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
    nullptr, nullptr, nullptr);
  if (png_ptr == nullptr){
    return PngResult::ERROR_CREATE_READ_STRUCT;
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == nullptr){
    return PngResult::ERROR_CREATE_INFO_STRUCT;
  }

  if (setjmp(png_jmpbuf(png_ptr))){
    // Fixme: Pass end_info
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return PngResult::ERROR_INIT_IO;
  }

  png_init_io(png_ptr, f);
  png_set_sig_bytes(png_ptr, 8);
  png_read_info(png_ptr, info_ptr);

  png_textp textPtr;
  int numText;
  if (png_get_text(png_ptr, info_ptr, &textPtr, &numText) > 0){
    for (int i = 0; i != numText; i++){
      // Fixme: Store somehow
    }
  }

  *width = png_get_image_width(png_ptr, info_ptr);
  *height = png_get_image_height(png_ptr, info_ptr);

  *colorType = png_get_color_type(png_ptr, info_ptr);
  *bitDepth = png_get_bit_depth(png_ptr, info_ptr);

  png_set_interlace_handling(png_ptr); // Fixme: returned num_passes irrelevant?
  png_read_update_info(png_ptr, info_ptr);

  if (setjmp(png_jmpbuf(png_ptr))){
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    return PngResult::ERROR_READ_DATA;
  }

  (*rowPointers) = (png_bytep*) malloc(sizeof(png_bytep) * (*height));
  for (png_uint_32 y = 0; y < (*height); y++){
    (*rowPointers)[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));
  }

  png_read_image(png_ptr, *rowPointers);
  // Fixme: Read post-image-data

  fclose(f);
  png_destroy_read_struct(&png_ptr, nullptr, nullptr); // Maybe
  return PngResult::OK;
}

static void free_rows(png_bytep* rowPointers, png_uint_32 height){
  for (png_uint_32 y = 0; y != height; y++){
    free(rowPointers[y]);
  }
  free(rowPointers);
}

OrError<Bitmap> read_png(const FilePath& path){
  png_byte** rowPointers = nullptr;
  png_uint_32 width;
  png_uint_32 height;
  png_byte colorType;
  png_byte bitDepth;
  PngResult result = read_with_libpng(path.Str().c_str(),
    &rowPointers,
    &width,
    &height,
    &colorType,
    &bitDepth);

  if (result != PngResult::OK){
    return to_string(result);
  }

  if (bitDepth != 8){
    // Fixme: Handle different bit-depths
    free_rows(rowPointers, height);
    return {"Unsupported bit depth"};
  }
  if (colorType != PNG_COLOR_TYPE_RGB_ALPHA){
    // Fixme: Handle different byte-orders
    free_rows(rowPointers, height);
    return {"Unsuppored png color-type"};
  }

  if (!can_represent<IntSize::value_type>(width)){
    return {"Unsupported png-width"};
  }
  if (!can_represent<IntSize::value_type>(height)){
    return {"Unsupported png-height"};
  }

  try {
    Bitmap bmp(IntSize(static_cast<int>(width), static_cast<int>(height)));
    auto* p = bmp.GetRaw();
    auto stride = bmp.GetStride();

    for (png_uint_32 y = 0; y < height; y++){
      for (png_uint_32 x = 0; x < width; x++){
        auto i =  y * stride + x * static_cast<png_uint_32>(faint::BPP);
        p[i + faint::iR] = rowPointers[y][x * 4];
        p[i + faint::iG] = rowPointers[y][x * 4 + 1];
        p[i + faint::iB] = rowPointers[y][x * 4 + 2];
        p[i + faint::iA] = rowPointers[y][x * 4 + 3];
      }
    }
    free_rows(rowPointers, height);
    return bmp;
  }
  catch (const BitmapException& e){
    free_rows(rowPointers, height);
    return {e.what()};
  }
}

} // namespace
