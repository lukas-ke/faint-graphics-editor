// -*- coding: us-ascii-unix -*-
// Copyright 2013 Lukas Kemmer
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

#ifndef FAINT_FILE_GIF_HH
#define FAINT_FILE_GIF_HH
#include <vector>
#include "bitmap/alpha-map.hh"
#include "bitmap/quantize.hh"
#include "formats/save-result.hh"
#include "util/delay.hh"

namespace faint{

class FilePath;
class ImageProps;

void read_gif(const FilePath&, ImageProps&);

struct GifFrame{
  GifFrame(const MappedColors& image, const Delay& delay)
    : image(image), delay(delay)
  {}
  MappedColors image;
  Delay delay;
};

SaveResult write_gif(const FilePath&, const std::vector<GifFrame>&);

} // namespace

#endif
