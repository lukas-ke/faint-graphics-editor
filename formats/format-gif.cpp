// -*- coding: us-ascii-unix -*-
// Copyright 2012 Lukas Kemmer
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

#include <algorithm>
#include <sstream>
#include "app/canvas.hh"
#include "formats/format.hh"
#include "formats/gif/file-gif.hh"
#include "text/formatting.hh"
#include "util/generator-adapter.hh"
#include "util/image.hh"
#include "util/image-util.hh"
#include "util/index-iter.hh"
#include "util/frame-iter.hh"
#include "util/make-vector.hh"

namespace faint{

static auto find_mismatch(const std::vector<IntSize>& sizes){
  return find_if_iter(sizes, not_equal_to(sizes.front()));
}

static bool uniform_size(const std::vector<IntSize>& sizes){
  return find_mismatch(sizes) == sizes.end();
}

static SaveResult fail_size_mismatch(const std::vector<IntSize>& sizes){
  size_t index = std::distance(begin(sizes), find_mismatch(sizes));
  assert(index != sizes.size());
  std::stringstream ss;
  ss << "This image can not be saved as a gif." << std::endl << std::endl <<
    "It contains frames of different sizes." << std::endl <<
    "Frame 1: " << str(sizes[0]) << std::endl <<
    "Frame " << index + 1 << ": " << str(sizes[index]);
  return SaveResult::SaveFailed(utf8_string(ss.str()));
}

static std::vector<IntSize> get_frame_sizes(Canvas& canvas){
  const auto get_size = [](const auto& frame){return frame.GetSize();};
  return make_vector(canvas, get_size);
}

class FormatGIF : public Format {
public:
  FormatGIF()
    : Format(FileExtension("gif"),
      label_t(utf8_string("Graphics Interchange Format (GIF)")),
      can_save(true),
      can_load(true))
  {}

  void Load(const FilePath& filePath, ImageProps& imageProps) override{
    read_gif(filePath, imageProps);
  }

  SaveResult Save(const FilePath& filePath, Canvas& canvas) override{
    // Verify that all frames have the same size
    auto sizes = get_frame_sizes(canvas);
    if (!uniform_size(sizes)){
      return fail_size_mismatch(sizes);
    }

    const auto adapt_frame =
      [](const auto& f) -> MappedColors_and_delay{
        return {quantized(flatten(f), Dithering::ON), f.GetDelay()};
      };

    return write_gif(filePath, make_vector(canvas, adapt_frame));
  }
};

Format* format_gif(){
  return new FormatGIF();
}

} // namespace
