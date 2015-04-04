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

#include "app/canvas.hh"
#include "formats/format.hh"
#include "formats/format-util.hh" // add_frame_or_set_error
#include "formats/png/file-png.hh"
#include "util/image-props.hh"
#include "util/image-util.hh" // flatten

namespace faint{

inline PngColorType alpha_if_necessary(const Bitmap& bmp){
  return fully_opaque(bmp) ? PngColorType::RGB : PngColorType::RGB_ALPHA;
}

class FormatPNG : public Format{
public:
  FormatPNG()
    : Format(FileExtension("png"),
      label_t("Portable network graphics (*.png)"),
      can_save(true),
      can_load(true))
  {}

  void Load(const FilePath& filePath, ImageProps& imageProps) override{
    add_frame_or_set_error(read_png(filePath), imageProps);
  }

  SaveResult Save(const FilePath& filePath, Canvas& canvas) override{
    Bitmap bmp(flatten(canvas.GetImage()));
    return write_png(filePath, bmp, alpha_if_necessary(bmp));
  }
};

Format* format_png(){
  return new FormatPNG();
}

} // namespace
