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

#include <cassert>
#include "text/utf8.hh"

namespace faint{namespace utf8{

inline unsigned int to_uint(const char ch){
  return static_cast<unsigned int>(static_cast<unsigned char>(ch));
}

size_t prefix_num_bytes(char cs){
  const unsigned char c(static_cast<unsigned char>(cs));

  if ((c & 0x80) == 0){
    return 1;
  }
  else if ((c & 0x40) == 0){
    assert(false); // Continuation (10xxxxxx) - this is not the leading byte and lacks length information.
  }
  else if ((c & 0xe0) == 0xc0){
    return 2;
  }
  else if ((c & 0xf0) == 0xe0){
    return 3;
  }
  else if ((c & 0xf8) == 0xf0){
    return 4;
  }
  assert(false);
  return 0;
}

size_t codepoint_num_bytes(unsigned int cp){
  if (cp <= 0x7f){
    return 1;
  }
  else if (cp <= 0x7ff){
    return 2;
  }
  else if (cp <= 0xffff){
    return 3;
  }
  else if (cp <= 0x1fffff){
    return 4;
  }
  else{
    assert(false);
    return 0;
  }
}

static size_t byte_num_to_char_num(size_t byte, const char* data,
  size_t maxBytes)
{
  size_t curByte = 0;
  size_t charNum = 0;
  const char* ptr = data;

  while (curByte < maxBytes && curByte != byte){
    size_t delta = prefix_num_bytes(*ptr);
    ptr += delta;
    curByte += delta;
    charNum++;
  }
  assert(curByte == byte);
  return charNum;
}

size_t byte_num_to_char_num(size_t byte, const std::string& data){
  return byte_num_to_char_num(byte, data.c_str(), data.size());
}

static size_t char_num_to_byte_num(size_t caret, const char* utf8,
  size_t maxBytes)
{
  assert(caret != std::string::npos);
  size_t charNum = 0;
  size_t offset = 0;
  const char* ptr = utf8;

  while (offset < maxBytes && charNum < caret){
    size_t delta = prefix_num_bytes(*ptr);
    ptr += delta;
    offset += delta;
    charNum++;
  }
  assert(charNum == caret);
  return offset;
}

size_t char_num_to_byte_num(size_t caret, const std::string& str){
  assert(caret <= num_characters(str));
  return char_num_to_byte_num(caret, str.c_str(), str.size());
}

unsigned int byte_string_to_codepoint(const std::string& ch){
  unsigned int c0 = to_uint(ch[0]);
  if ((c0 & 0x80) == 0){
    return c0 & 0x7f;
  }
  else if ((c0 & 0xe0) == 0xc0){
    unsigned int value = (c0 & 0x1f) << 6;
    value |= (to_uint(ch[1]) & 0x3f);
    return value;
  }
  else if ((c0 & 0xf0) == 0xe0){
    unsigned int value = (c0 & 0xf) << 12;
    value |= (to_uint(ch[1]) & 0x3f) << 6;
    value |= (to_uint(ch[2])) & 0x3f;
    return value;
  }
  else if ((c0 & 0xf8) == 0xf0){
    return
      ((c0 & 0x7) << 18) |
      ((to_uint(ch[1]) & 0x3f) << 12) |
      ((to_uint(ch[2]) & 0x3f) << 6) |
      ((to_uint(ch[3]) & 0x3f));
  }
  assert(false);
  return 0;
}

std::string codepoint_to_byte_string(unsigned int cp){
  const unsigned int continuation = 0x80;
  std::string ch;
  if (cp <= 0x7f){
    ch += std::string(1, static_cast<char>((unsigned char)(cp)));
    assert(ch.size() == 1);
  }
  else if (cp <= 0x7ff){
    const unsigned int lead_2 = 0xc0;
    ch += static_cast<char>((unsigned char)(((cp >> 6) & 0x1f) | lead_2));
    ch += static_cast<char>((unsigned char)((cp & 0x3f) | continuation));
  }
  else if (cp <= 0xffff){
    const unsigned int lead_3 = 0xe0;
    ch += static_cast<char>((unsigned char)(((cp >> 12) & 0xf) | lead_3));
    ch += static_cast<char>((unsigned char)(((cp >> 6) & 0x3f) | continuation));
    ch += static_cast<char>((unsigned char)((cp & 0x3f) | continuation));
  }
  else if (cp <= 0x1fffff){
    const unsigned int lead_4 = 0xf0;
    ch += static_cast<char>((unsigned char)(((cp >> 18) & 0xf) | lead_4));
    ch += static_cast<char>((unsigned char)(((cp >> 12) & 0x3f) | continuation));
    ch += static_cast<char>((unsigned char)(((cp >> 6) & 0x3f) | continuation));
    ch += static_cast<char>((unsigned char)((cp & 0x3f) | continuation));
  }
  else {
    // 5 and 6 byte sequences removed by RFC 3629
    assert(false);
  }
  return ch;
}

size_t num_characters(const std::string& utf8){
  const size_t maxBytes = utf8.size();
  size_t numChars = 0;
  size_t numBytes = 0;
  const char* ptr = utf8.c_str();
  while (numBytes < maxBytes){
    size_t delta = prefix_num_bytes(*ptr);
    ptr += delta;
    numBytes += delta;
    assert(numBytes <= maxBytes);
    numChars++;
  }
  return numChars;
}

}} // namespace
