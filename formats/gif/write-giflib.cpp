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
#include "formats/gif/giflib-5.0.5/lib/gif_lib.h"
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
  const auto& b1 = v.front().image;
  const auto& image = std::get<AlphaMap>(b1);
  const auto& colorList = std::get<ColorList>(b1);
  const auto size = image.GetSize(); // TODO: Max gif-size?

  // Fixme: Use delay
  // Fixme: Support mask color

  if (colorList.GetNumColors() > 256){
    return GifWriteResult::ERROR_TOO_LARGE_PALETTE;
  }

  GifFile gifFile(path);
  if (gifFile.f == nullptr){
    return GifWriteResult::ERROR_OPEN_FILE;
  }

  std::unique_ptr<GifColorType[]> colorPtr(
    new GifColorType[256]);

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

  // Fixme: Legacy, according to gif_lib.h
  auto err = EGifPutScreenDesc(gifFile.f,
    size.w,
    size.h,
    colorMap.BitsPerPixel,
    0,
    &colorMap);
  if (err == GIF_ERROR){
    return GifWriteResult::ERROR_OTHER;
  }

  err = EGifPutImageDesc(gifFile.f,
    0,
    0, size.w, size.h, false, nullptr);
  if (err == GIF_ERROR){
    return GifWriteResult::ERROR_OTHER;
  }

  for (int y = 0; y != size.h; y++){
    auto scanline = const_cast<GifPixelType*>(image.GetRaw() + y * size.w);
    err = EGifPutLine(gifFile.f, scanline, size.w);
    if (err == GIF_ERROR){
      return GifWriteResult::ERROR_OTHER;
    }
  }

  return GifWriteResult::OK;
}

} // namespace
