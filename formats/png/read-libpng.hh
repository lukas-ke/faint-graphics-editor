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

#ifndef FAINT_READ_LIBPNG_HH
#define FAINT_READ_LIBPNG_HH
#include <png.h> // libpng
#include "formats/png/file-png.hh"

namespace faint{

enum class PngReadResult{
  OK,
  ERROR_OPEN_FILE,
  ERROR_PNG_SIGNATURE,
  ERROR_CREATE_READ_STRUCT,
  ERROR_CREATE_INFO_STRUCT,
  ERROR_INIT_IO,
  ERROR_READ_DATA,
  ERROR_READ_PALETTE,
  ERROR_MALLOC
};

PngReadResult read_with_libpng(const FilePath&,
  png_byte** rows,
  png_uint_32* width,
  png_uint_32* height,
  png_byte* colorType,
  png_byte* bitDepth,
  int* bitsPerPixel,
  std::vector<png_color>& palette,
  std::map<utf8_string, utf8_string>& textChunks);

} // namespace

#endif
