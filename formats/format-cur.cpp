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

#include "app/canvas.hh"
#include "app/frame-iter.hh"
#include "formats/bmp/file-cur.hh"
#include "formats/format.hh"
#include "util/image-props.hh"
#include "util/image-util.hh"
#include "util/image.hh"
#include "util/make-vector.hh"

namespace faint{

static auto to_cursor(const Image& frame){
  return std::pair(flatten(frame), frame.GetHotSpot());
}

class FormatCUR : public Format {
public:
  FormatCUR()
    : Format(FileExtension("cur"),
      label_t(utf8_string("Windows cursor (CUR)")),
      can_save(true),
      can_load(true))
  {}

  void Load(const FilePath& filename, ImageProps& imageProps){
    read_cur(filename).Visit(
      [&](cur_vec& cursors){
        for (auto& [bmp, hotspot] : cursors){
          imageProps.AddFrame(std::move(bmp), FrameInfo(hotspot));
        }
      },
      [&](const utf8_string& error){
        imageProps.SetError(error);
      });
  }

  SaveResult Save(const FilePath& filePath, Canvas& canvas){
    return write_cur(filePath, make_vector(canvas, to_cursor));
  }
};

Format* format_cur(){
  return new FormatCUR();
}

} // namespace
