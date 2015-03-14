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
#include "text/utf8-string.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/file-path.hh"

namespace faint{

static wxBitmap from_any(const char* data, size_t len, wxBitmapType type){
  wxMemoryInputStream stream(data, len);
  wxImage image(stream, type);
  if (!image.IsOk()){
    // Fixme: Add proper, propagating error handling
    return wxBitmap(10,10);
  }
  if (image.HasMask()){
    image.InitAlpha();
  }
  return wxBitmap(image);
}

Bitmap from_jpg(const char* jpg, size_t len){
  return to_faint(from_any(jpg, len, wxBITMAP_TYPE_JPEG));
}

Bitmap from_png(const char* png, size_t len){
  return to_faint(from_any(png, len, wxBITMAP_TYPE_PNG));
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
  auto faceNames(wxFontEnumerator().GetFacenames());
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

} // namespace

namespace faint{namespace mouse{

IntPoint screen_position(){
  return to_faint(wxGetMousePosition());
}

IntPoint view_position(const wxWindow& w){
  return to_faint(w.ScreenToClient(wxGetMousePosition()));
}

Point image_position(const CanvasGeo& g, const wxWindow& w){
  Point p(floated(view_position(w)));
  const coord zoom = g.zoom.GetScaleFactor();
  return Point((p.x + g.pos.x - g.border.w) / zoom,
    (p.y + g.pos.y - g.border.w) / zoom);
}

}} // namespace
