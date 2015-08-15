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

#include "util-wx/slice-wx.hh"
#include "text/slice-generic.hh"

namespace faint{

wxString char_at(const wxString& s, int pos){
  return generic::char_at(s, pos);
}

wxString slice(const wxString& s, int from, int up_to){
  return generic::slice(s, from, up_to);
}

wxString slice_from(const wxString& s, int first){
  return generic::slice_from(s, first);
}

wxString slice_up_to(const wxString& s, int up_to){
  return generic::slice_up_to(s, up_to);
}

} // namespace
