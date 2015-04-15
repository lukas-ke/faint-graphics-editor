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
#include "bitmap/bitmap-exception.hh"
#include "formats/png/file-png.hh"
#include "formats/png/png-util.hh"
#include "formats/png/write-libpng.hh"
#include "geo/limits.hh"
#include "text/formatting.hh"
#include "python/py-interface.hh" // Fixme

#ifdef _MSC_VER
#pragma warning(disable:4611) // _setjmp and C++-object destruction
#endif

namespace faint{

int to_png_color_type(PngColorType c){
  switch (c){
  case PngColorType::RGB:
    return PNG_COLOR_TYPE_RGB;

  case PngColorType::RGB_ALPHA:
    return PNG_COLOR_TYPE_RGB_ALPHA;

  case PngColorType::GRAY:
    return PNG_COLOR_TYPE_GRAY;

  case PngColorType::GRAY_ALPHA:
    return PNG_COLOR_TYPE_GRAY_ALPHA;
  }
  assert(false);
  return -1;
}


enum class PngReadResult{
  OK,
  ERROR_OPEN_FILE,
  ERROR_PNG_SIGNATURE,
  ERROR_CREATE_READ_STRUCT,
  ERROR_CREATE_INFO_STRUCT,
  ERROR_INIT_IO,
  ERROR_READ_DATA,
  ERROR_READ_PALETTE, // Fixme: Add error handling for this
  ERROR_MALLOC
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
  else if (result == R::ERROR_MALLOC){
    return failed_read_libpng("Out of memory.");
  }
  else{
    return failed_read("For reasons unknown.");
  }
}

static PngReadResult read_with_libpng(const char* path,
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

  // --
  // Output parameters
  png_byte* rows = nullptr;
  png_uint_32 width;
  png_uint_32 height;
  png_byte colorType;
  png_byte bitDepth;
  int pngBitsPerPixel;
  png_tEXt_map textChunks;
  png_color* palette = nullptr;
  int numPalette = 0;

  PngReadResult result = read_with_libpng(path.Str().c_str(),
    &rows,
    &width,
    &height,
    &colorType,
    &bitDepth,
    &pngBitsPerPixel,
    &palette,
    &numPalette,
    textChunks);
  // --

  if (result != PngReadResult::OK){
    return to_string(result, path);
  }

  if (bitDepth != 8){
    free(rows);
    return {"Unsupported bit depth: " + str_int(bitDepth)};
  }

  if (!rgb_or_rgba(colorType) && !gray_or_gray_alpha(colorType) &&
    !palettized(colorType))
  {
    // Fixme: Handle all color types
    free(rows);
    return {"Unsuppored png color-type: " + color_type_to_string(colorType)};
  }

  if (colorType == PNG_COLOR_TYPE_GRAY){
    if (pngBitsPerPixel != 8){
      return {"Unsupported bits-per-pixel for PNG_COLOR_TYPE GRAY: " +
          str_int(pngBitsPerPixel)};
    }
  }
  else if (colorType == PNG_COLOR_TYPE_GRAY_ALPHA){
    if (pngBitsPerPixel != 16){
      return {"Unsupported bits-per-pixel for PNG_COLOR_TYPE_GRAY_ALPHA: " +
          str_int(pngBitsPerPixel)};
    }
  }
  else if (colorType == PNG_COLOR_TYPE_PALETTE){
    if (pngBitsPerPixel != 8){
      free(palette);
      return {"Unsupported bits-per-pixel for PNG_COLOR_TYPE_PALETTE; " +
          str_int(pngBitsPerPixel)};
    }
  }
  else if (colorType == PNG_COLOR_TYPE_RGB_ALPHA){
    if (pngBitsPerPixel != 32 && pngBitsPerPixel != 24){
      return {"Unsupported bits-per-pixel: " + str_int(pngBitsPerPixel)};
    }
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

    if (gray_or_gray_alpha(colorType)){
      // Read GRAY

      for (png_uint_32 y = 0; y < height; y++){
        const auto* row = rows + y * width * PNG_BPP;
        for (png_uint_32 x = 0; x < width; x++){
          auto i =  y * stride + x * BMP_BPP;
          const png_byte v = row[x * PNG_BPP];
          p[i + faint::iR] = v;
          p[i + faint::iG] = v;
          p[i + faint::iB] = v;
          p[i + faint::iA] = (colorType == PNG_COLOR_TYPE_GRAY_ALPHA) ?
            row[x * PNG_BPP + 1] : faint::CHANNEL_MAX;
        }
      }
    }
    else if (rgb_or_rgba(colorType)){
      // Read RGB or RGBA

      for (png_uint_32 y = 0; y < height; y++){
        const auto* row = rows + y * width * PNG_BPP;
        for (png_uint_32 x = 0; x < width; x++){
          auto i =  y * stride + x * BMP_BPP;
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
    }
    else if (palettized(colorType)){
      python_print("Palettized.");
      for (png_uint_32 y = 0; y < height; y++){
        const auto* row = rows + y * width * PNG_BPP;

        for (png_uint_32 x = 0; x < width; x++){
          auto i =  y * stride + x * BMP_BPP;
          auto paletteIndex = row[x * PNG_BPP]; // PNG_BPP Should be 1 probably.
          auto color = palette[paletteIndex];

          p[i + faint::iR] = color.red;
          p[i + faint::iG] = color.green;
          p[i + faint::iB] = color.blue;
          p[i + faint::iA] = 255;
        }
      }
      free(palette);
    }

    free(rows);
    return {Bitmap_and_tEXt(std::move(bmp), std::move(textChunks))};
  }
  catch (const BitmapException& e){
    free(rows);
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


static utf8_string to_string(PngWriteResult result, const FilePath& p){
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

SaveResult write_png(const FilePath& path,
  const Bitmap& bmp,
  PngColorType colorType,
  const png_tEXt_map& textChunks)
{
  PngWriteResult result = write_with_libpng(path.Str().c_str(), bmp,
    to_png_color_type(colorType),
    textChunks);

  return result == PngWriteResult::OK ?
    SaveResult::SaveSuccessful() :
    SaveResult::SaveFailed(to_string(result, path));
}

SaveResult write_png(const FilePath& path,
  const Bitmap& bmp,
  PngColorType colorType)
{
  png_tEXt_map noChunks;
  return write_png(path, bmp, colorType, noChunks);
}

const utf8_string get_libpng_version(){
  return utf8_string(PNG_LIBPNG_VER_STRING);
}

} // namespace
