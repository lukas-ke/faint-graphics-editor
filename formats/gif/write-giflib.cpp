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
#include "formats/faint-fopen.hh"
#include "formats/gif/write-giflib.hh"
#include "formats/gif/giflib-5.0.5/gif_lib.h"
#include "util/iter.hh"
#include "util-wx/file-path.hh"
#include <sys/stat.h>
#include <fcntl.h>

namespace {

class GifFile{
  // Wrapper for automatically calling EGifCloseFile on a GifFileType*
public:
  explicit GifFile(const faint::FilePath& path) {
    auto handle = faint_open(path,
      O_WRONLY | O_CREAT | O_TRUNC,
      S_IREAD | S_IWRITE);

    f = (handle != -1) ?
      EGifOpenFileHandle(handle, nullptr) :
      nullptr;
  }

  ~GifFile(){
    if (f != nullptr){
      EGifCloseFile(f);
    }
  }

  bool Bad() const{
    return f == nullptr;
  }

  operator GifFileType*(){
    return f;
  }

private:
  GifFileType* f;
};

} // namespace

namespace faint{

GifWriteResult write_with_giflib(const FilePath& path,
  const std::vector<GifFrame>& v)
{
  assert(!v.empty());

  GifFile gifFile(path);
  if (gifFile.Bad()){
    return GifWriteResult::ERROR_OPEN_FILE;
  }

  // For writing the screen descriptor once based on the first image
  // Fixme: Rework, write the first and then use but_last or smth
  bool first = true;
  static const int PALETTE_LENGTH = 256;

  for (const auto& entry : v){
    const auto& map = entry.image.map;
    const auto size = map.GetSize(); // Fixme: Max gif-size?
    if (first){
      const auto& globalColorList = v.front().image.palette;
      if (globalColorList.size() > 256){
        return GifWriteResult::ERROR_TOO_LARGE_PALETTE;
      }

      // Fixme: Must be some multiple when used with ColorMapObject?
      std::unique_ptr<GifColorType[]> colorPtr(
        new GifColorType[PALETTE_LENGTH]);

      for (const auto& p : enumerate(globalColorList)){
        const auto i = p.num;
        const auto& c = p.item;
        colorPtr[i].Red = c.r;
        colorPtr[i].Green = c.g;
        colorPtr[i].Blue = c.b;
      }

      // GIF89, for animation and transparency support
      EGifSetGifVersion(gifFile, true);

      ColorMapObject colorMap;
      colorMap.ColorCount = PALETTE_LENGTH;
      colorMap.BitsPerPixel = 8; // Fixme: ?
      colorMap.SortFlag = false; // Fixme: ?
      colorMap.Colors = colorPtr.get();

      auto err = EGifPutScreenDesc(gifFile,
        size.w,
        size.h,
        colorMap.BitsPerPixel,
        entry.image.transparencyIndex.Or(0), // Background
        &colorMap);
      if (err == GIF_ERROR){
        return GifWriteResult::ERROR_OTHER;
      }
    }

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

    err = EGifPutExtension(gifFile, GRAPHICS_EXT_FUNC_CODE, 4, extension);
    if (err == GIF_ERROR){
      return GifWriteResult::ERROR_OTHER;
    }

    const auto& colorList = entry.image.palette;
    std::unique_ptr<GifColorType[]> colorPtr(new GifColorType[PALETTE_LENGTH]);

    for (const auto& p : enumerate(colorList)){
      const auto i = p.num;
      const auto& c = p.item;
      colorPtr[i].Red = c.r;
      colorPtr[i].Green = c.g;
      colorPtr[i].Blue = c.b;
    }

    // Fixme: Duplicates screen-descriptor (global color map)
    ColorMapObject colorMap;
    colorMap.ColorCount = PALETTE_LENGTH; // Fixme: Must be some multiple?
    colorMap.BitsPerPixel = 8; // Fixme: ?
    colorMap.SortFlag = false; // Fixme: ?
    colorMap.Colors = colorPtr.get();

    // Avoid specifying a palette for the first image - it would be
    // identical to the global color map.
    auto colorMapPtr = first ? nullptr : &colorMap;

    err = EGifPutImageDesc(gifFile,
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
      err = EGifPutLine(gifFile, scanline, size.w);
      if (err == GIF_ERROR){
        return GifWriteResult::ERROR_OTHER;
      }
    }
    first = false;
  }
  return GifWriteResult::OK;
}

} // namespace
