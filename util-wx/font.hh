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

#ifndef FAINT_FONT_HH
#define FAINT_FONT_HH
#include <vector>
#include "bitmap/color.hh"
#include "text/utf8-string.hh"

namespace faint{

utf8_string get_default_font_name();
int get_default_font_size();
std::vector<utf8_string> available_font_facenames();
bool valid_facename(const utf8_string&);
Color get_highlight_color();

} // namespace

#endif
