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

#ifndef FAINT_IMAGE_INFO_HH
#define FAINT_IMAGE_INFO_HH
#include "bitmap/color.hh"
#include "geo/int-size.hh"
#include "util/distinct.hh"

namespace faint{

class category_imageinfo;
using create_bitmap = Distinct<bool, category_imageinfo, 0>;

class ImageInfo {
public:
  explicit ImageInfo(const IntSize&, const create_bitmap&);
  ImageInfo(const IntSize&, const Color& bg, const create_bitmap&);
  IntSize size;
  Color backgroundColor;
  bool createBitmap;
};

} // namespace

#endif
