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

#include <cctype> // toupper
#include "text/utf8.hh"
#include "text/utf8-char.hh"

namespace faint {

inline unsigned int to_uint(const char ch){
  return static_cast<unsigned int>(static_cast<unsigned char>(ch));
}

bool surrogate_codepoint(unsigned int cp){
  return 0xd800 <= cp && cp <= 0xdfff;
}

utf8_char::utf8_char(const std::string& u8char)
  : m_cp(utf8::byte_string_to_codepoint(u8char))
{}

size_t utf8_char::bytes() const{
  return utf8::codepoint_num_bytes(m_cp);
}

unsigned int utf8_char::codepoint() const{
  return m_cp;
}

std::string utf8_char::str() const{
  return utf8::codepoint_to_byte_string(m_cp);
}

bool utf8_char::operator==(const utf8_char& other) const{
  return m_cp == other.m_cp;
}

bool utf8_char::operator!=(const utf8_char& other) const{
  return m_cp != other.m_cp;
}

bool utf8_char::operator<(const utf8_char& other) const{
  return m_cp < other.m_cp;
}

utf8_char toupper(const utf8_char& ch){
  if (ch.bytes() == 1){
    return utf8_char(std::string(1,
      static_cast<char>(std::toupper(ch.str().back()))));
  }

  // Only ascii-range supported. Return unchanged.
  return ch;
}

utf8_char tolower(const utf8_char& ch){
  if (ch.bytes() == 1){
    return utf8_char(std::string(1,
      static_cast<char>(std::tolower(ch.str().back()))));
  }

  // Only ascii-range supported. Return unchanged.
  return ch;
}

bool isalpha(const utf8_char& ch){
  int cp = ch.codepoint();
  return
    (65 <= cp && cp <= 90) ||
    (97 <= cp && cp <= 122);
}

bool isdigit(const utf8_char& ch){
  int cp = ch.codepoint();
  return 48 <= cp && cp <= 57;
}

} // namespace
