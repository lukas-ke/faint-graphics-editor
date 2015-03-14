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

#ifndef FAINT_UTF8_HH
#define FAINT_UTF8_HH
#include <string>

namespace faint{namespace utf8{

size_t prefix_num_bytes(char);
size_t codepoint_num_bytes(unsigned int);
size_t num_characters(const std::string&);

size_t byte_num_to_char_num(size_t, const std::string&);
size_t byte_num_to_char_num(size_t, const char*) = delete;

size_t char_num_to_byte_num(size_t, const std::string&);
size_t char_num_to_byte_num(size_t, const char*) = delete;

unsigned int byte_string_to_codepoint(const std::string&);

std::string codepoint_to_byte_string(unsigned int);

}} // namespace

#endif
