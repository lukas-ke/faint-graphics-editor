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

#ifndef FAINT_UTF8_CHAR_HH
#define FAINT_UTF8_CHAR_HH
#include <string>

namespace faint{

class utf8_char{
public:
  explicit utf8_char(const std::string&);
  explicit constexpr utf8_char(unsigned int codePoint);
  unsigned int codepoint() const;
  size_t bytes() const;
  std::string str() const;
  bool operator==(const utf8_char&) const;
  bool operator!=(const utf8_char&) const;
  bool operator<(const utf8_char&) const;
private:
  unsigned int m_cp;
};

constexpr utf8_char::utf8_char(unsigned int cp): m_cp(cp){}

// True if the given codepoint is a surrogate-pair reserved for UTF-16
// (hence, invalid UTF-8).
bool surrogate_codepoint(unsigned int);

utf8_char toupper(const utf8_char&);
utf8_char tolower(const utf8_char&);

// Note: Recognizes only ascii alphabetic characters
bool isalpha(const utf8_char&);
bool isdigit(const utf8_char&);

} // namespace

#endif
