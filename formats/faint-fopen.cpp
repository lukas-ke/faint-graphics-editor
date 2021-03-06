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

#include "wx/string.h"
#include "faint-fopen.hh"
#include "text/utf8-string.hh"
#include "util-wx/convert-wx.hh"

#ifdef _MSC_VER // Windows

namespace faint{

FILE* faint_fopen_write_binary(const FilePath& path){
  const std::wstring filename_u16 = iostream_friendly(path);
  return _wfopen(filename_u16.c_str(), L"wb");
}

FILE* faint_fopen_read_binary(const FilePath& path){
  const std::wstring filename_u16 = iostream_friendly(path);
  return _wfopen(filename_u16.c_str(), L"rb");
}

int faint_open(const FilePath& path, int oflag, int pmode){
  const std::wstring filename_u16 = iostream_friendly(path);
  return _wopen(filename_u16.c_str(), oflag, pmode);
}

} // namespace

#else // Non-windows

namespace faint{

#include <sys/stat.h>
#include <fcntl.h>

FILE* faint_fopen_write_binary(const FilePath& path){
  return fopen(path.Str().c_str(), "wb");
}

FILE* faint_fopen_read_binary(const FilePath& path){
  return fopen(path.Str().c_str(), "rb");
}

int faint_open(const FilePath& path, int oflag, int pmode){
  return open(path.Str().c_str(), oflag, pmode);
}

} // namespace

#endif
