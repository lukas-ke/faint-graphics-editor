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

#ifndef FAINT_FOPEN_HH
#define FAINT_FOPEN_HH
#include <cstdio>
#include "util-wx/file-path.hh"

namespace faint {

// Handles utf-8 also on Windows, MSVC:s fopen does not.
FILE* faint_fopen_write_binary(const FilePath&);

// Handles utf-8 also on Windows, MSVC:s open does not.
int faint_open(const FilePath&, int oflag, int pmode);

} // namespace

#endif
