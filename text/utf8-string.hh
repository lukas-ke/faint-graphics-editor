// -*- coding: us-ascii-unix -*-
// Copyright 2012 Lukas Kemmer
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

#ifndef FAINT_UTF8_STRING_HH
#define FAINT_UTF8_STRING_HH
#include "text/utf8-char.hh"
#include "text/utf8-string-iterator.hh"

namespace faint {

class utf8_string{
public:
  static const size_t npos;
  utf8_string() = default;
  utf8_string(size_t n, const utf8_char&);
  explicit utf8_string(const std::string&);
  explicit utf8_string(const utf8_char&);
  utf8_string(const char*);

  size_t bytes() const;
  utf8_string substr(size_t pos, size_t n=std::string::npos) const;
  const std::string& str() const;
  const char* c_str() const;
  void clear();

  // The number of code points in the string
  size_t size() const;
  utf8_string& erase(size_t, size_t n=npos);
  bool empty() const;
  size_t find(const utf8_char&, size_t start=0) const;
  size_t rfind(const utf8_char&, size_t start=npos) const;
  utf8_string& insert(size_t, const utf8_string&);
  utf8_string& insert(size_t, size_t, const utf8_char&);
  utf8_string& operator+=(const utf8_char&);
  utf8_string& operator+=(const utf8_string&);
  utf8_string& operator=(const utf8_string&);
  utf8_char operator[](size_t) const;
  bool operator<(const utf8_string&) const;
private:
  std::string m_data;
};

bool is_ascii(const utf8_string&);
utf8_string operator+(const utf8_string&, const utf8_char&);
utf8_string operator+(const utf8_char&, const utf8_string&);
utf8_string operator+(const utf8_string&, const utf8_string&);
std::ostream& operator<<(std::ostream&, const utf8_string&);
bool operator==(const utf8_string&, const utf8_string&);
bool operator!=(const utf8_string&, const utf8_string&);

utf8_string_const_iterator begin(const utf8_string&);
utf8_string_const_iterator end(const utf8_string&);

} // namespace

#endif
