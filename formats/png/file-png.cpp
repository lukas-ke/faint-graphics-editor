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

static utf8_string to_string(PngReadResult result, const FilePath& p){
  using R = PngReadResult;

  auto failed_read = [](const utf8_string& s){
    return endline_sep("Failed reading png.\n", s);
  };

  auto failed_read_libpng = [&](const utf8_string& func){
    return failed_read(space_sep("libpng", quoted(func), "exited with error."));
  };

  if (result == R::OK){
    return "No error";
  }
  else if (result == R::ERROR_OPEN_FILE){
    return failed_read(endline_sep("File could not be opened for reading.",
      space_sep("File:", p.Str())));
  }
  else if (result == R::ERROR_PNG_SIGNATURE){
    return failed_read("Incorrect PNG signature.");
  }
  else if (result == R::ERROR_CREATE_READ_STRUCT){
    return failed_read_libpng("create_read_struct");
  }
  else if (result == R::ERROR_CREATE_INFO_STRUCT){
    return failed_read_libpng("png_create_info_struct");
  }
  else if (result == R::ERROR_INIT_IO){
    return failed_read_libpng("png_init_io");
  }
  else if (result == R::ERROR_READ_DATA){
    return failed_read_libpng("png_read_image");
  }
  else{
    return failed_read("For reasons unknown.");
  }
}

PngReadResult read_with_libpng(const char* path,
  png_bytep** rowPointers,
  png_uint_32* width,
  png_uint_32* height,
  png_byte* colorType,
  png_byte* bitDepth,
  int* bitsPerPixel,
  std::map<utf8_string, utf8_string>& textChunks)
{
  FILE* f = fopen(path, "rb");
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

  (*rowPointers) = (png_bytep*) malloc(sizeof(png_bytep) * (*height));
  for (png_uint_32 y = 0; y < (*height); y++){
    (*rowPointers)[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));
  }

  png_read_image(png_ptr, *rowPointers);
  // Fixme: Read post-image-data

  png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
  fclose(f);
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

OrError<Bitmap_and_tEXt> read_png_meta(const FilePath& path){
  png_byte** rowPointers = nullptr;
  png_uint_32 width;
  png_uint_32 height;
  png_byte colorType;
  png_byte bitDepth;
  int pngBitsPerPixel;
  png_tEXt_map textChunks;
  PngReadResult result = read_with_libpng(path.Str().c_str(),
    &rowPointers,
    &width,
    &height,
    &colorType,
    &bitDepth,
    &pngBitsPerPixel,
    textChunks);

  if (result != PngReadResult::OK){
    return to_string(result, path);
  }

  if (bitDepth != 8){
    // Fixme: Handle all bit-depths
    free_rows(rowPointers, height);

    return {"Unsupported bit depth: " + str_int(bitDepth)};
  }

  if (colorType != PNG_COLOR_TYPE_RGB_ALPHA && colorType != PNG_COLOR_TYPE_RGB){
    // Fixme: Handle all color types
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
    return {Bitmap_and_tEXt(std::move(bmp), std::move(textChunks))};
  }
  catch (const BitmapException& e){
    free_rows(rowPointers, height);
    return {e.what()};
  }
}

OrError<Bitmap> read_png(const FilePath& p){
  return read_png_meta(p).Visit(
    [](const Bitmap_and_tEXt& result){
      return OrError<Bitmap>(result.bmp);
    },
    [](const utf8_string& error){
      return OrError<Bitmap>(error);
    });
}

enum class PngWriteResult{
  OK,
  ERROR_OPEN_FILE,
  ERROR_CREATE_WRITE_STRUCT,
  ERROR_CREATE_INFO_STRUCT,
  ERROR_INIT_IO,
  ERROR_WRITE_HEADER,
  ERROR_WRITE_DATA,
  ERROR_WRITE_END,
  ERROR_WRITE_TEXT_KEY_ENCODING,
  ERROR_WRITE_TEXT_VALUE_ENCODING,
  ERROR_WRITE_TEXT_KEY_EMPTY,
  ERROR_WRITE_TEXT_KEY_TOO_LONG,
  ERROR_WRITE_TEXT_VALUE_TOO_LONG,
  ERROR_WRITE_TOO_MANY_TEXT_CHUNKS
};

const utf8_string to_string(PngWriteResult result, const FilePath& p){
  // Fixme: Compare error text style with bmp-save errors etc
  using R = PngWriteResult;

  auto failed_write = [](const utf8_string& s){
    return endline_sep("Failed saving png.\n", s);
  };

  auto failed_write_libpng = [&](const utf8_string& func){
    return failed_write(space_sep("libpng", quoted(func), "exited with error."));
  };


  if (result == R::OK){
    return "No error";
  }
  else if (result == R::ERROR_OPEN_FILE){
    return failed_write(endline_sep("File could not be opened for writing.",
      space_sep("File:", p.Str())));
  }
  else if (result == R::ERROR_CREATE_WRITE_STRUCT){
    return failed_write_libpng("png_create_write_struct");
  }
  else if (result == R::ERROR_INIT_IO){
    return failed_write_libpng("png_init_io");
  }
  else if (result == R::ERROR_WRITE_HEADER){
    return failed_write_libpng("png_write_info");
  }
  else if (result == R::ERROR_WRITE_DATA){
    return failed_write_libpng("png_write_image");
  }
  else if (result == R::ERROR_WRITE_END){
    return failed_write_libpng("png_write_end");
  }
  else if (result == R::ERROR_WRITE_TEXT_KEY_ENCODING){
    return failed_write(endline_sep("A specified tEXt-chunk key was not ascii.",
      "Faint only supports ascii tEXt-chunks."));
  }
  else if (result == R::ERROR_WRITE_TEXT_KEY_EMPTY){
    return failed_write("An empty key was specified for a tEXt-chunk.");
  }
  else if (result == R::ERROR_WRITE_TEXT_KEY_TOO_LONG){
    return failed_write("Keys for tEXt-chunks may be at most " +
      str_int(PNG_KEYWORD_MAX_LENGTH) + "-characters");
  }
  else if (result == R::ERROR_WRITE_TEXT_VALUE_TOO_LONG){
    return failed_write("A value for a tEXt-chunks exceeded capacity of "
      "png_size_t.");
  }
  else{
    return failed_write("For reasons unknown.");
  }
}

PngWriteResult write_with_libpng(const char* path,
  const Bitmap& bmp,
  const png_tEXt_map& textChunks)
{
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

SaveResult write_png(const FilePath& path,
  const Bitmap& bmp,
  const png_tEXt_map& textChunks)
{
  PngWriteResult result = write_with_libpng(path.Str().c_str(), bmp, textChunks);
  return result == PngWriteResult::OK ?
    SaveResult::SaveSuccessful() :
    SaveResult::SaveFailed(to_string(result, path));
}

SaveResult write_png(const FilePath& path, const Bitmap& bmp){
  png_tEXt_map noChunks;
  return write_png(path, bmp, noChunks);
}


} // namespace
