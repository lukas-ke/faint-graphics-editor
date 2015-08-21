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

#include "util/image-info.hh"

namespace faint{

ImageInfo::ImageInfo(const IntSize& size, const create_bitmap& createBitmap)
  : size(size),
    backgroundColor(color_white),
    createBitmap(createBitmap.Get())
{}

ImageInfo::ImageInfo(const IntSize& size,
  const Color& bgCol,
  const create_bitmap& createBitmap)
  : size(size),
    backgroundColor(bgCol),
    createBitmap(createBitmap.Get())
{}

} // namespace
