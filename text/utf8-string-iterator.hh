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

#ifndef FAINT_UTF8_STRING_ITERATOR_HH
#define FAINT_UTF8_STRING_ITERATOR_HH
#include <iterator>
#include <memory>
#include "text/utf8.hh" // fixme
#include "text/utf8-char.hh" // fixme

namespace faint{

class utf8_string;

class utf8_string_const_iterator{
public:
  using value_type = utf8_char;
  using reference = const utf8_char&;
  using pointer = std::unique_ptr<utf8_char>;
  using difference_type = size_t;
  using iterator_category = std::input_iterator_tag;
  utf8_string_const_iterator(const utf8_string&, size_t);
  utf8_char operator*() const;
  utf8_string_const_iterator& operator++();
  bool operator==(const utf8_string_const_iterator&) const;
  bool operator!=(const utf8_string_const_iterator&) const;
  utf8_string_const_iterator& operator=(const utf8_string_const_iterator&);
private:
  // Fixme: store the raw data instead for faster iteration
  const utf8_string* m_str;
  size_t m_index;
};

} // namespace

#endif
