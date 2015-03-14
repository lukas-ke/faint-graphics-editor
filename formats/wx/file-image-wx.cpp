// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
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

#include "bitmap/bitmap.hh"
#include "formats/wx/file-image-wx.hh"
#include "text/formatting.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/file-path.hh"
#include "util-wx/scoped-error-log.hh"

namespace faint{

utf8_string label_for_wx_image(wxBitmapType type){
  if (type == wxBITMAP_TYPE_PNG){
    return "Portable Network Graphics";
  }
  else if (type == wxBITMAP_TYPE_JPEG){
    return "JPEG";
  }
  else if (type == wxBITMAP_TYPE_BMP){
    return "Bitmap 24bit (*.bmp)";
  }
  else {
    return "Unknown bitmap type";
  }
}

OrError<Bitmap> read_image_wx(const FilePath& filePath, wxBitmapType bmpType){
  assert(bmpType != wxBITMAP_TYPE_GIF);

  // Collect wxWidgets log errors
  ScopedErrorLog logger;

  // I must load like this (i.e. create a wxImage, init alpha and
  // convert to Bitmap) to retain alpha channel. See wxWidgets ticket
  // #3019
  wxImage image(to_wx(filePath.Str()), bmpType);
  if (!image.IsOk()){
    return logger.GetMessages();
  }
  if (image.HasMask() && !image.HasAlpha()){
    image.InitAlpha();
  }
  wxBitmap bmp(image);
  return to_faint(bmp);
}

SaveResult write_image_wx(const Bitmap& bmp,
  wxBitmapType bmpType,
  const FilePath& path)
{
  wxImage imageWx(to_wx_image(bmp));

  ScopedErrorLog logger;
  bool saveOk = imageWx.SaveFile(to_wx(path.Str()), bmpType);

  if (saveOk){
    return SaveResult::SaveSuccessful();
  }
  else {
    // Fixme: wxImage::SaveFile does not seem to reliably return false
    // on failure (Seen when attempting to overwrite locked .jpg on
    // Windows). Also this seems to trash the locked file?
    return SaveResult::SaveFailed(endline_sep(
      space_sep("Failed saving as", quoted(label_for_wx_image(bmpType)) + "."),
      logger.GetMessages()));
  }
}

} // namespace
