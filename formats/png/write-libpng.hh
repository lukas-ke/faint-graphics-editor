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

#ifndef FAINT_WRITE_LIBPNG_HH
#define FAINT_WRITE_LIBPNG_HH
#include "formats/png/file-png.hh"

namespace faint{

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

PngWriteResult write_with_libpng(const char* path,
  const Bitmap&,
  const int colorType,
  const png_tEXt_map& textChunks);

} // namespace faint

#endif
