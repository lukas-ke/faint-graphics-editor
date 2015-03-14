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

#include <sstream>
#include "wx/arrstr.h"
#include "wx/string.h"
#include "text/utf8-string.hh"
#include "util-wx/clipboard.hh"
#include "util-wx/clipboard-util.hh"
#include "util-wx/convert-wx.hh"

namespace faint{

static utf8_string serialize_size(const Size& size){
  std::stringstream ss;
  ss << size.w << "," << size.h;
  return to_faint(wxString(ss.str()));
}

static Optional<Size> deserialize_size(const utf8_string& str){
  wxString wxStr(to_wx(str));
  wxArrayString strs = wxSplit(to_wx(str), ',', '\0');
  if (strs.GetCount() != 2){
    return no_option();
  }

  long width;
  if (!strs[0].ToLong(&width)){
    return no_option();
  }
  if (width <= 0){
    return no_option();
  }

  long height;
  if (!strs[1].ToLong(&height)){
    return no_option();
  }
  if (height <= 0){
    return no_option();
  }

  return option(Size(static_cast<coord>(width), static_cast<coord>(height))); // Fixme: Check casts
}

Optional<Size> clipboard_get_size(){
  Clipboard clip;
  if (!clip.Good()){
    return no_option();
  }
  else if (auto maybeStr = clip.GetText()){
    return deserialize_size(maybeStr.Get());
  }

  return no_option();
}

void clipboard_copy_size(const Size& size){
  Clipboard clip;
  if (clip.Good()){
    clip.SetText(serialize_size(size));
  }
}

} // namespace
