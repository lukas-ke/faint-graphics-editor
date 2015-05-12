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

#ifndef FAINT_WRITE_GIFLIB_HH
#define FAINT_WRITE_GIFLIB_HH
#include "formats/gif/file-gif.hh"

namespace faint{

enum class GifWriteResult{
  OK,
  ERROR_OPEN_FILE,
  ERROR_OUT_OF_MEMORY,
  ERROR_OTHER,
  ERROR_TOO_LARGE_PALETTE
};

GifWriteResult write_with_giflib(const char* path,
  const std::vector<GifFrame>&);

} // namespace

#endif
