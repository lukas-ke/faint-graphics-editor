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
#include "formats/bmp/file-cur.hh"
#include "formats/format.hh"
#include "util/image.hh"
#include "util/image-props.hh"
#include "util/image-util.hh"
#include "util/index-iter.hh"

namespace faint{

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
        for (auto& c : cursors){
          imageProps.AddFrame(std::move(c.first), FrameInfo(c.second));
        }
      },
      [&](const utf8_string& error){
        imageProps.SetError(error);
      });
  }

  SaveResult Save(const FilePath& filePath, Canvas& canvas){
    cur_vec cursors;
    for (auto i : up_to(canvas.GetNumFrames())){
      const Image& frame(canvas.GetFrame(i));
      cursors.emplace_back(std::make_pair(flatten(frame), frame.GetHotSpot()));
    }
    return write_cur(filePath, cursors);
  }
};

Format* format_cur(){
  return new FormatCUR();
}

} // namespace
