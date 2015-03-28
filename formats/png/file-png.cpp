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
#include "text/formatting.hh"

#ifdef _MSC_VER
#pragma warning(disable:4611) // _setjmp and C++-object destruction
#endif

namespace faint{

enum class PngReadResult{
  OK,
  ERROR_OPEN_FILE,
  ERROR_PNG_SIGNATURE,
  ERROR_CREATE_READ_STRUCT,
  ERROR_CREATE_INFO_STRUCT,
  ERROR_INIT_IO,
  ERROR_READ_DATA
};

static utf8_string to_string(PngReadResult result){
  // Fixme: Compare error text style with bmp-load errors etc
  using R = PngReadResult;
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
    return "Failed initializing IO"; // Fixme: Memory? Png init?
  }
  else if (result == R::ERROR_READ_DATA){
    return "Failed reading image data.";
  }
  else{
    return "Unknown error.";
  }
}

PngReadResult read_with_libpng(const char* path,
  png_bytep** rowPointers,
  png_uint_32* width,
  png_uint_32* height,
  png_byte* colorType,
  png_byte* bitDepth,
  png_byte* bitsPerPixel)
{
  FILE* f = fopen(path, "rb");
  if (f == nullptr){
    return PngReadResult::ERROR_OPEN_FILE;
  }

  png_byte sig[8];
  size_t readBytes = fread(sig, 1, 8, f);
  if (readBytes != 8){
    return PngReadResult::ERROR_PNG_SIGNATURE;
  }

  if (!png_check_sig(sig, 8)){
    return PngReadResult::ERROR_PNG_SIGNATURE;
  }

  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
    nullptr, nullptr, nullptr);
  if (png_ptr == nullptr){
    return PngReadResult::ERROR_CREATE_READ_STRUCT;
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (info_ptr == nullptr){
    return PngReadResult::ERROR_CREATE_INFO_STRUCT;
  }

  if (setjmp(png_jmpbuf(png_ptr))){
    // Fixme: Pass end_info
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    return PngReadResult::ERROR_INIT_IO;
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
  *bitsPerPixel = *bitDepth * png_get_channels(png_ptr, info_ptr);

  png_set_interlace_handling(png_ptr); // Fixme: returned num_passes irrelevant?
  png_read_update_info(png_ptr, info_ptr);

  if (setjmp(png_jmpbuf(png_ptr))){
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    return PngReadResult::ERROR_READ_DATA;
  }

  (*rowPointers) = (png_bytep*) malloc(sizeof(png_bytep) * (*height));
  for (png_uint_32 y = 0; y < (*height); y++){
    (*rowPointers)[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));
  }

  png_read_image(png_ptr, *rowPointers);
  // Fixme: Read post-image-data

  fclose(f);
  png_destroy_read_struct(&png_ptr, nullptr, nullptr); // Maybe
  return PngReadResult::OK;
}

static void free_rows(png_bytep* rowPointers, png_uint_32 height){
  for (png_uint_32 y = 0; y != height; y++){
    free(rowPointers[y]);
  }
  free(rowPointers);
}

static utf8_string color_type_to_string(png_byte colorType){
  if (colorType == PNG_COLOR_TYPE_GRAY){
    return "PNG_COLOR_TYPE_GRAY";
  }
  else if (colorType == PNG_COLOR_TYPE_PALETTE){
    return "PNG_COLOR_TYPE_PALETTE";
  }
  else if (colorType == PNG_COLOR_TYPE_RGB){
    return "PNG_COLOR_TYPE_RGB";
  }
  else if (colorType == PNG_COLOR_TYPE_RGB_ALPHA){
    return "PNG_COLOR_TYPE_ALPHA";
  }
  else if (colorType == PNG_COLOR_TYPE_GRAY_ALPHA){
    return "PNG_COLOR_TYPE_GRAY_ALPHA";
  }
  else{
    return space_sep("UNKNOWN", bracketed(str_int(colorType)));
  }
}

OrError<Bitmap> read_png(const FilePath& path){
  png_byte** rowPointers = nullptr;
  png_uint_32 width;
  png_uint_32 height;
  png_byte colorType;
  png_byte bitDepth;
  png_byte pngBitsPerPixel;
  PngReadResult result = read_with_libpng(path.Str().c_str(),
    &rowPointers,
    &width,
    &height,
    &colorType,
    &bitDepth,
    &pngBitsPerPixel);

  if (result != PngReadResult::OK){
    return to_string(result);
  }

  if (bitDepth != 8){
    // Fixme: Handle different bit-depths
    free_rows(rowPointers, height);

    return {"Unsupported bit depth: " + str_int(bitDepth)};
  }
  if (colorType != PNG_COLOR_TYPE_RGB_ALPHA && colorType != PNG_COLOR_TYPE_RGB){
    // Fixme: Handle different byte-orders
    free_rows(rowPointers, height);
    return {"Unsuppored png color-type: " + color_type_to_string(colorType)};
  }

  if (pngBitsPerPixel != 32 && pngBitsPerPixel != 24){
    return {"Unsupported bits-per-pixel: " + str_int(pngBitsPerPixel)};
  }

  if (!can_represent<IntSize::value_type>(width)){
    return {"Unsupported png-width"};
  }
  if (!can_represent<IntSize::value_type>(height)){
    return {"Unsupported png-height"};
  }

  try {
    // Read the data into a faint-Bitmap
    Bitmap bmp(IntSize(static_cast<int>(width), static_cast<int>(height)));
    auto* p = bmp.GetRaw();
    const png_uint_32 stride = convert(bmp.GetStride());
    const png_uint_32 BMP_BPP = convert(faint::BPP);
    const png_uint_32 PNG_BPP = convert(pngBitsPerPixel / 8);

    for (png_uint_32 y = 0; y < height; y++){
      for (png_uint_32 x = 0; x < width; x++){
        auto i =  y * stride + x * BMP_BPP;
        const auto* row = rowPointers[y];
        p[i + faint::iR] = row[x * PNG_BPP];
        p[i + faint::iG] = row[x * PNG_BPP + 1];
        p[i + faint::iB] = row[x * PNG_BPP + 2];
        if (PNG_BPP == 3){
          p[i + faint::iA] = 255;
        }
        else{
          p[i + faint::iA] = row[x * PNG_BPP + 3];
        }
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

enum class PngWriteResult{
  OK,
  ERROR_OPEN_FILE,
  ERROR_CREATE_WRITE_STRUCT,
  ERROR_CREATE_INFO_STRUCT,
  ERROR_INIT_IO,
  ERROR_WRITE_HEADER,
  ERROR_WRITE_DATA,
  ERROR_WRITE_END
};

const utf8_string to_string(PngWriteResult result){
  // Fixme: Compare error text style with bmp-save errors etc
  using R = PngWriteResult;
  if (result == R::OK){
    return "No error.";
  }
  else if (result == R::ERROR_OPEN_FILE){
    return "Failed opening file for writing.";
  }
  else if (result == R::ERROR_CREATE_WRITE_STRUCT){
    return "Failed creating write structure"; // Fixme: Memory? Png init?
  }
  else if (result == R::ERROR_INIT_IO){
    return "Failed failed initializing IO"; // Fixme: Memory? Png init?
  }
  else if (result == R::ERROR_WRITE_HEADER){
    return "Failed writing header"; // Fixme: Memory? Png init?
  }
  else if (result == R::ERROR_WRITE_DATA){
    return "Failed writing image data";
  }
  else if (result == R::ERROR_WRITE_END){
    return "Failed writing trailer"; // Fixme: Maybe not trailer
  }
  else{
    return "Unknown error.";
  }
}

PngWriteResult write_with_libpng(const char* path, const Bitmap& bmp){
  assert(bitmap_ok(bmp));
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

  bool alpha = !fully_opaque(bmp);
  int colorType = alpha ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB;
  const int PNG_BPP = alpha ? 4 : 3;

  png_set_IHDR(png_ptr,
    info_ptr,
    width,
    height,
    bitDepth,
    colorType,
    PNG_INTERLACE_NONE,
    PNG_COMPRESSION_TYPE_BASE,
    PNG_FILTER_TYPE_BASE);

  png_write_info(png_ptr, info_ptr);

  // Create raw-data arrays for libpng from the Bitmap.
  png_byte** rowPointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
  const auto bytesPerRow = png_get_rowbytes(png_ptr, info_ptr);
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

  // Write the image data
  if (setjmp(png_jmpbuf(png_ptr))){
    fclose(f);
    return PngWriteResult::ERROR_WRITE_DATA;
  }
  png_write_image(png_ptr, rowPointers);

  // Write end
  if (setjmp(png_jmpbuf(png_ptr))){
    free_rows(rowPointers, height);
    return PngWriteResult::ERROR_WRITE_END;
  }
  png_write_end(png_ptr, nullptr);

  free_rows(rowPointers, height);
  fclose(f);
  return PngWriteResult::OK;
}

SaveResult write_png(const FilePath& path, const Bitmap& bmp){
  PngWriteResult result = write_with_libpng(path.Str().c_str(), bmp);
  return result == PngWriteResult::OK ?
    SaveResult::SaveSuccessful() :
    SaveResult::SaveFailed(to_string(result));
}

} // namespace
