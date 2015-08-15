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

#ifndef FAINT_SLICE_UTF8_HH
#define FAINT_SLICE_UTF8_HH
#include "text/utf8-string.hh"

namespace faint{

// See slice-generic.hh for documentation

utf8_string char_at(const utf8_string&, int pos);
utf8_string slice(const utf8_string&, int, int);
utf8_string slice_from(const utf8_string&, int);
utf8_string slice_up_to(const utf8_string&, int);

std::string char_at(const char*, int pos);
std::string slice(const char*, int, int);
std::string slice_from(const char*, int);
std::string slice_up_to(const char*, int);

std::string char_at(const std::string&, int);
std::string slice(const std::string&, int, int);
std::string slice_from(const std::string&, int);
std::string slice_up_to(const std::string&, int);

} // namespace

#endif
