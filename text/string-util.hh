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

#ifndef FAINT_STRING_UTIL_HH
#define FAINT_STRING_UTIL_HH
#include "text/utf8-string.hh"
#include "util/distinct.hh"

namespace faint{

class category_checks;
using with_t = Distinct<utf8_string, category_checks, 1>;
inline with_t with(const utf8_string& s){
  return with_t(s);
}

bool ends(const utf8_string&, const with_t&);

} // namespace

#endif
