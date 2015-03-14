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

#include <map>
#include "wx/bitmap.h"
#include "app/canvas.hh"
#include "bitmap/bitmap.hh"
#include "formats/file-formats.hh"
#include "formats/format.hh"
#include "formats/wx/file-image-wx.hh"
#include "util/image-props.hh"
#include "util/image-util.hh"

namespace faint{

class FormatWX : public Format {
public:
  FormatWX(const std::vector<FileExtension>& exts,
    wxBitmapType type)
    : Format(exts,
      label_t(label_for_wx_image(type)),
      can_save(true),
      can_load(true)),
      m_bmpType(type)
  {
    assert(type != wxBITMAP_TYPE_GIF);
  }

  void Load(const FilePath& filePath, ImageProps& imageProps) override{
    read_image_wx(filePath, m_bmpType).Visit(
      [&](Bitmap& bmp){
        imageProps.AddFrame(std::move(bmp), FrameInfo());
      },
      [&](const utf8_string& error){
        imageProps.SetError(error);
      });
  }

  SaveResult Save(const FilePath& filePath, Canvas& canvas) override{
    const Image& image(canvas.GetImage());
    Bitmap bmp(flatten(image));
    return write_image_wx(bmp, m_bmpType, filePath);
  }

private:
  wxBitmapType m_bmpType;
};

Format* format_wx_jpg(){
  return new FormatWX({FileExtension("jpg"), FileExtension("jpeg")},
    wxBITMAP_TYPE_JPEG);
}

Format* format_wx_png(){
  return new FormatWX({FileExtension("png")}, wxBITMAP_TYPE_PNG);
}

} // namespace
