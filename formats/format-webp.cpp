// -*- coding: us-ascii-unix -*-
// Copyright 2020 Lukas Kemmer
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
#include "formats/format.hh"
#include "formats/format-util.hh" // add_frame_or_set_error
#include "formats/webp/file-webp.hh"

namespace faint{

class FormatWebP : public Format {
public:
  FormatWebP()
    : Format(FileExtension("webp"),
        label_t("WebP"),
        can_save(false), // Fixme
        can_load(true))
  {}

  void Load(const FilePath& filePath, ImageProps& imageProps) override{
    add_frame_or_set_error(read_webp(filePath), imageProps);
  }

  SaveResult Save(const FilePath&, Canvas&) override{
    return SaveResult::SaveFailed("Not implemented"); // Fixme
  }
};

Format* format_webp(){
  return new FormatWebP();
}

} // namespace
