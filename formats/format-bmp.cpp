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

#include <cassert>
#include "app/canvas.hh"
#include "bitmap/bitmap.hh"
#include "formats/format.hh"
#include "formats/bmp/file-bmp.hh"
#include "util/image-props.hh"
#include "util/image-util.hh"

namespace faint{

static label_t get_bmp_label(BitmapQuality quality){
  switch (quality){
  case BitmapQuality::COLOR_8BIT:
    return label_t(utf8_string("Bitmap 8bit (*.bmp)"));

  case BitmapQuality::COLOR_24BIT:
    return label_t(utf8_string("Bitmap 24bit (*.bmp)"));

  case BitmapQuality::GRAY_8BIT:
    return label_t(utf8_string("Bitmap 8bit gray (*.bmp)"));
  }

  assert(false);
  return label_t(utf8_string(""));
}

class FormatBMP : public Format{
public:
  FormatBMP(BitmapQuality quality)
    : Format(FileExtension("bmp"),
      get_bmp_label(quality),
      can_save(true),
      can_load(false)),
      m_quality(quality)
  {}

  FormatBMP()
    : Format(FileExtension("bmp"),
      label_t("Windows Bitmap (*.bmp)"),
      can_save(false),
      can_load(true))
  {}

  void Load(const FilePath& filePath, ImageProps& imageProps) override{
    read_bmp(filePath).Visit(
      [&](Bitmap& bmp){
        imageProps.AddFrame(std::move(bmp), FrameInfo());
      },
      [&](const utf8_string& s){
        imageProps.SetError(s);
      });
  }

  SaveResult Save(const FilePath& filePath, Canvas& canvas) override{
    assert(m_quality.IsSet());
    Bitmap bmp(flatten(canvas.GetImage()));
    return write_bmp(filePath, bmp, m_quality.Get());
  }

private:
  Optional<BitmapQuality> m_quality;
};

Format* format_load_bmp(){
  return new FormatBMP();
}

Format* format_save_bmp(BitmapQuality quality){
  return new FormatBMP(quality);
}

} // namespace
