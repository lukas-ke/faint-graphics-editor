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

#ifndef FAINT_GUI_STRING_TYPES_HH
#define FAINT_GUI_STRING_TYPES_HH
#include "text/utf8-string.hh"
#include "util/distinct.hh"

namespace faint{

class category_gui_string;
using Title = Distinct<utf8_string, category_gui_string, 0>;
using storage_name = Distinct<utf8_string, category_gui_string, 1>;
using Tooltip = Distinct<utf8_string, category_gui_string, 3>;
using Description = Distinct<utf8_string, category_gui_string, 4>;

} // namespace

#endif
