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

#include "bitmap/bitmap-exception.hh"
#include "formats/png/file-png.hh"
#include "formats/png/png-util.hh"
#include "formats/png/read-libpng.hh"
#include "formats/png/write-libpng.hh"
#include "geo/limits.hh"
#include "text/formatting.hh"

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
  else if (result == R::ERROR_READ_PALETTE){
    return failed_read_libpng("png_get_PLTE");
  }
  else{
    return failed_read("For reasons unknown.");
  }
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

  PngReadResult result = read_with_libpng(path,
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

  if (bitDepth != 8 && bitDepth != 16){
    free(rows);
    return {"Unsupported bit depth: " + str_int(bitDepth)};
  }

  if (!rgb_or_rgba(colorType) && !gray_or_gray_alpha(colorType) &&
    !palettized(colorType))
  {
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
    if (pngBitsPerPixel != 64 &&
      pngBitsPerPixel != 32 &&
      pngBitsPerPixel != 24)
    {
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
    const auto bmpStride = bmp.GetStride();
    const auto bmpByPP = ByPP;
    const png_uint_32 PNG_ByPP = convert(pngBitsPerPixel / 8);

    if (gray_or_gray_alpha(colorType)){
      // Read GRAY

      for (png_uint_32 y = 0; y < height; y++){
        const auto* row = rows + y * width * PNG_ByPP;
        for (png_uint_32 x = 0; x < width; x++){
          auto i =  y * bmpStride + x * bmpByPP;
          const png_byte v = row[x * PNG_ByPP];
          p[i + faint::iR] = v;
          p[i + faint::iG] = v;
          p[i + faint::iB] = v;
          p[i + faint::iA] = (colorType == PNG_COLOR_TYPE_GRAY_ALPHA) ?
            row[x * PNG_ByPP + 1] : faint::CHANNEL_MAX;
        }
      }
    }
    else if (rgb_or_rgba(colorType)){

      if (PNG_ByPP == 3 || PNG_ByPP == 4){
        // Read RGB or RGBA
        for (png_uint_32 y = 0; y < height; y++){
          const auto* row = rows + y * width * PNG_ByPP;
          for (png_uint_32 x = 0; x < width; x++){
            auto i =  y * bmpStride + x * bmpByPP;
            p[i + faint::iR] = row[x * PNG_ByPP];
            p[i + faint::iG] = row[x * PNG_ByPP + 1];
            p[i + faint::iB] = row[x * PNG_ByPP + 2];
            if (PNG_ByPP == 3){
              p[i + faint::iA] = 255;
            }
            else{
              p[i + faint::iA] = row[x * PNG_ByPP + 3];
            }
          }
        }
      }
      else if (PNG_ByPP == 8){
        // Note: Discards the least significant byte for each channel,
        // since Faint-bitmaps use 4 ByPP.
        for (png_uint_32 y = 0; y < height; y++){
          const auto* row = rows + y * width * PNG_ByPP;
          for (png_uint_32 x = 0; x < width; x++){
            auto i =  y * bmpStride + x * bmpByPP;
            p[i + faint::iR] = row[x * PNG_ByPP + 1];
            p[i + faint::iG] = row[x * PNG_ByPP + 3];
            p[i + faint::iB] = row[x * PNG_ByPP + 5];
            p[i + faint::iA] = row[x * PNG_ByPP + 7];
          }
        }
      }
    }
    else if (palettized(colorType)){
      for (png_uint_32 y = 0; y < height; y++){
        const auto* row = rows + y * width * PNG_ByPP;

        for (png_uint_32 x = 0; x < width; x++){
          auto i =  y * bmpStride + x * bmpByPP;
          auto paletteIndex = row[x * PNG_ByPP]; // PNG_ByPP Should be 1 probably.
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
  PngWriteResult result = write_with_libpng(path, bmp,
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
