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

#ifndef FAINT_ENCODE_BITMAP_HH
#define FAINT_ENCODE_BITMAP_HH
#include <string>

namespace faint{

class Bitmap;

Bitmap from_jpg(const char*, size_t len); // Implemented in util-wx.cpp
Bitmap from_png(const char*, size_t len); // Implemented in util-wx.cpp

// Returns a string representing the Bitmap encoded as a PNG
std::string to_png_string(const Bitmap&); // Implemented in util-wx.cpp

} // namespace

#endif
