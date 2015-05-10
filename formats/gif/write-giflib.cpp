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

#include <memory>
#include "formats/gif/write-giflib.hh"
#include "formats/gif/giflib-5.0.5/gif_lib.h"
#include "util/iter.hh"

namespace {

class GifFile{
  // Wrapper for automatically calling EGifCloseFile on a GifFileType*
public:
  GifFile(const char* path){
    // Fixme: Throw on error
    int openErr;
    f = EGifOpenFileName(path, false, &openErr);
  }
  ~GifFile(){
    if (f != nullptr){
      EGifCloseFile(f);
    }
  }
  GifFileType* f;
};

} // namespace

namespace faint{

GifWriteResult write_with_giflib(const char* path,
  const std::vector<MappedColors_and_delay>& v)
{
  assert(!v.empty());

  GifFile gifFile(path);
  if (gifFile.f == nullptr){
    return GifWriteResult::ERROR_OPEN_FILE;
  }

  // For writing the screen descriptor once based on the first image
  // Fixme: Rework, write the first and then use but_last or smth
  bool first = true;
  const bool isGif89 = v.size() > 1;

  for (const auto& entry : v){
    const auto& map = entry.image.map;
    const auto size = map.GetSize(); // TODO: Max gif-size?
    // Fixme: Support mask color

    if (first){
      const auto& globalColorList = v.front().image.palette;
      if (globalColorList.GetNumColors() > 256){
        return GifWriteResult::ERROR_TOO_LARGE_PALETTE;
      }
      std::unique_ptr<GifColorType[]> colorPtr(
        new GifColorType[256]);

      for (const auto& p : enumerate(globalColorList)){
        const auto i = p.num;
        const auto& c = p.item;
        colorPtr[i].Red = c.r;
        colorPtr[i].Green = c.g;
        colorPtr[i].Blue = c.b;
      }

      ColorMapObject colorMap;
      colorMap.ColorCount = 256; // Fixme: Must be some multiple?
      colorMap.BitsPerPixel = 8; // Fixme: ?
      colorMap.SortFlag = false; // Fixme: ?
      colorMap.Colors = colorPtr.get();

      EGifSetGifVersion(gifFile.f, isGif89); // GIF89, for animation support
      // Fixme: Legacy API, according to gif_lib.h
      auto err = EGifPutScreenDesc(gifFile.f,
        size.w,
        size.h,
        colorMap.BitsPerPixel,
        entry.image.transparencyIndex.Or(0), // Background, fixme?
        &colorMap);
      if (err == GIF_ERROR){
        return GifWriteResult::ERROR_OTHER;
      }
    }

    if (isGif89) {
      GraphicsControlBlock gcb;
      gcb.DisposalMode = DISPOSE_BACKGROUND;
      gcb.UserInputFlag = false;
      gcb.DelayTime = entry.delay.Get();
      gcb.TransparentColor = entry.image.transparencyIndex.Or(NO_TRANSPARENT_COLOR);

      GifByteType extension[4];
      auto err = EGifGCBToExtension(&gcb, extension);
      if (err == GIF_ERROR){
        return GifWriteResult::ERROR_OTHER;
      }

      err = EGifPutExtension(gifFile.f, GRAPHICS_EXT_FUNC_CODE, 4, extension);
      if (err == GIF_ERROR){
        return GifWriteResult::ERROR_OTHER;
      }
    }

    const auto& colorList = entry.image.palette;
    std::unique_ptr<GifColorType[]> colorPtr(new GifColorType[256]);

    for (const auto& p : enumerate(colorList)){
      const auto i = p.num;
      const auto& c = p.item;
      colorPtr[i].Red = c.r;
      colorPtr[i].Green = c.g;
      colorPtr[i].Blue = c.b;
    }

    ColorMapObject colorMap;
    colorMap.ColorCount = 256; // Fixme: Must be some multiple?
    colorMap.BitsPerPixel = 8; // Fixme: ?
    colorMap.SortFlag = false; // Fixme: ?
    colorMap.Colors = colorPtr.get();

    // Avoid specifying a palette for the first image - it would be
    // identical to the global color map.
    auto colorMapPtr = first ? nullptr : &colorMap;

    auto err = EGifPutImageDesc(gifFile.f,
      0, // GifLeft
      0, // GifTop
      size.w, // GifWidth
      size.h, // GifHeight
      false, // GifInterlace
      colorMapPtr);

    if (err == GIF_ERROR){
      return GifWriteResult::ERROR_OTHER;
    }

    // Write the pixel-data
    for (int y = 0; y != size.h; y++){
      auto scanline = const_cast<GifPixelType*>(map.GetRaw() + y * size.w);
      err = EGifPutLine(gifFile.f, scanline, size.w);
      if (err == GIF_ERROR){
        return GifWriteResult::ERROR_OTHER;
      }
    }
    first = false;
  }
  return GifWriteResult::OK;
}

} // namespace
