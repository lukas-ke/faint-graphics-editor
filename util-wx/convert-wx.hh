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

#ifndef FAINT_CONVERT_WX_HH
#define FAINT_CONVERT_WX_HH
#include <string>
#include <vector>
#include "wx/chartype.h"
#include "util/common-fwd.hh"

class wxArrayString;
class wxBitmap;
class wxColour;
class wxCursor;
class wxFileName;
class wxImage;
class wxKeyEvent;
class wxMouseEvent;
class wxPoint;
class wxRect;
class wxSize;
class wxString;

namespace faint{

wxFileName absoluted(const wxFileName&);

wxColour to_wx(const Color&);
wxColour to_wx(const ColRGB&);
wxRect to_wx(const IntRect&);
wxPoint to_wx(const IntPoint&);
wxSize to_wx(const IntSize&);

template<typename T>
auto to_wx(const std::vector<T>& v){
  using T2 = decltype(to_wx(v.front()));
  std::vector<T2> v2;
  v2.reserve(v.size());
  for (auto& o : v){
    v2.push_back(to_wx(o));
  }
  return v2;
}

// Converts a Bitmap to a wxBitmap.
// Note that wxImage (see to_wx_image) may be preferable, especially
// for wxImage::SaveFile.
wxBitmap to_wx_bmp(const Bitmap&);

// Convert a Bitmap to a wxImage.
//
// Note: wxImage seems less platform-dependent than wxBitmap. This is
// preferable, as it will cause less surprises depending on for
// example the native bit-depth support. Also when saving, wxBitmap in
// some cases converts to wxImage, so to_wx_image may be a more direct
// route.
wxImage to_wx_image(const Bitmap&);

Color to_faint(const wxColour&);
IntRect to_faint(const wxRect&);
IntPoint to_faint(const wxPoint&);
IntSize to_faint(const wxSize&);
Bitmap to_faint(wxBitmap);
FileList to_FileList(const wxArrayString&);

ToolModifiers get_tool_modifiers();
ToolModifiers mouse_modifiers(const wxMouseEvent&);
Mod key_modifiers(const wxKeyEvent&);

// Getting raw data for pasted bitmaps fails in MSW unless this
// function is called.
wxBitmap clean_bitmap(const wxBitmap& dirtyBmp);

utf8_char to_faint(const wxChar&);
utf8_string to_faint(const wxString&);

wxString to_wx(const utf8_string&);

wxString get_clipboard_text();
std::vector<wxString> wx_split_lines(const wxString&);

#ifdef FAINT_MSW
// In Visual Studios iostream implementation, char* filenames are
// expected to be ANSI, and wchar_t* are expected to be UTF-16.
std::wstring iostream_friendly(const wxString&);
#else
// Assume other platforms treat char* as UTF8
std::string iostream_friendly(const wxString&);
#endif

} // namespace

#endif
