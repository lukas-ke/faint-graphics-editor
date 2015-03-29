// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
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

#ifndef FAINT_FILE_PNG_HH
#define FAINT_FILE_PNG_HH
#include <map>
#include "bitmap/bitmap.hh"
#include "formats/save-result.hh"
#include "text/utf8-string.hh"
#include "util-wx/file-path.hh"
#include "util/or-error.hh"

namespace faint{

enum class PngColorType{
  RGB,
  RGB_ALPHA,
  GRAY,
  GRAY_ALPHA,
};

using png_tEXt_map = std::map<utf8_string, utf8_string>;

struct Bitmap_and_tEXt{
  Bitmap_and_tEXt(Bitmap&& bmp, png_tEXt_map&& text)
    : bmp(std::move(bmp)),
      text(std::move(text))
  {}

  Bitmap bmp;
  png_tEXt_map text;
};

// Reads a PNG-file to a Bitmap.
OrError<Bitmap> read_png(const FilePath&);

// Reads a PNG-file to a bitmap along with a map of the png-tEXT key,
// value pairs.
OrError<Bitmap_and_tEXt> read_png_meta(const FilePath&);

// Writes a Bitmap to a png-file.
SaveResult write_png(const FilePath&, const
  Bitmap&,
  PngColorType);

// Writes a Bitmap o a png-file, along with the tEXt chunks.
// The tEXt chunks are written before the image data.
SaveResult write_png(const FilePath&,
  const Bitmap&,
  PngColorType,
  const png_tEXt_map&);

} // namespace

#endif
