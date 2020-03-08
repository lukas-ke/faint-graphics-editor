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

#include "wx/bitmap.h"
#include "wx/filename.h"
#include "wx/font.h"
#include "wx/fontenum.h"
#include "wx/mstream.h"
#include "wx/settings.h"
#include "wx/stdpaths.h"
#include "wx/utils.h"
#include "wx/window.h"
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "geo/canvas-geo.hh"
#include "geo/geo-func.hh"
#include "geo/point.hh"
#include "geo/size.hh"
#include "text/utf8-string.hh"
#include "util/or-error.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/file-path.hh"
#include "util-wx/scoped-error-log.hh"

namespace faint{

static OrError<Bitmap> bmp_from_format(wxBitmapType type,
  const char* data,
  size_t len)
{
  ScopedErrorLog errorLog;

  wxMemoryInputStream stream(data, len);
  wxImage image(stream, type);
  if (!image.IsOk()){
    return {errorLog.GetMessages()};
  }
  if (image.HasMask()){
    image.InitAlpha();
  }
  return {to_faint(wxBitmap(image))};
}

OrError<Bitmap> from_jpg(const char* jpgData, size_t len){
  return bmp_from_format(wxBITMAP_TYPE_JPEG, jpgData, len);
}

OrError<Bitmap> from_png(const char* pngData, size_t len){
  return bmp_from_format(wxBITMAP_TYPE_PNG, pngData, len);
}

std::string to_png_string(const Bitmap& bmp){
  wxImage img_wx(to_wx_image(bmp));

  // Write the image as a png to a memory stream
  wxMemoryOutputStream stream;
  img_wx.SaveFile(stream, wxBITMAP_TYPE_PNG);
  const size_t length = stream.GetSize();

  // Copy the png bytes from that stream
  std::vector<char> buffer(length);
  stream.CopyTo(buffer.data(), length);
  return std::string(buffer.begin(), buffer.end());
}

std::vector<utf8_string> available_font_facenames(){
  auto faceNames = wxFontEnumerator().GetFacenames();
  std::vector<utf8_string> v;
  v.reserve(faceNames.size());
  for (const auto& faceName : faceNames){
    v.emplace_back(to_faint(faceName));
  }
  return v;
}

static wxFont get_default_font(){
  return wxFont(wxFontInfo(12));
}

utf8_string get_default_font_name(){
  static utf8_string defaultName(get_default_font().GetFaceName());
  return defaultName;
}

int get_default_font_size(){
  static int defaultSize(get_default_font().GetPointSize());
  return defaultSize;
}

bool valid_facename(const utf8_string& name){
  return wxFontEnumerator::IsValidFacename(to_wx(name));
}

Color get_highlight_color(){
  return to_faint(wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
}

DirPath get_home_dir(){
  return DirPath(wxGetHomeDir());
}

static wxFileName get_faint_dir_wx(){
  wxStandardPathsBase& paths = wxStandardPaths::Get();
  return absoluted(paths.GetExecutablePath()).GetPath();
}

FilePath get_faint_exe_path(){
  wxStandardPathsBase& paths = wxStandardPathsBase::Get();
  wxFileName filename = absoluted(paths.GetExecutablePath());
  return FilePath::FromAbsoluteWx(filename);
}

DirPath get_data_dir(){
  wxString pathStr(get_faint_dir_wx().GetFullPath());
  pathStr.Replace("\\", "/");
  return DirPath(pathStr);
}

} // namespace (faint)

namespace faint::mouse{

IntPoint screen_position(){
  return to_faint(wxGetMousePosition());
}

IntPoint view_position(const wxWindow& w){
  return to_faint(w.ScreenToClient(wxGetMousePosition()));
}

Point image_position(const CanvasGeo& g, const wxWindow& w){
  const IntPoint viewPos = view_position(w);
  const IntPoint scroll = g.pos;
  const IntPoint border = point_from_size(g.border);
  const coord zoom = g.zoom.GetScaleFactor();

  return (viewPos + scroll - border) / zoom;
}

} // namespace (faint::mouse)
