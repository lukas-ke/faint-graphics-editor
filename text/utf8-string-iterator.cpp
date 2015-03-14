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

#include "text/utf8-string-iterator.hh"
#include "text/utf8-string.hh"

namespace faint{

// Const iterator
utf8_string_const_iterator::utf8_string_const_iterator(const utf8_string& s,
  size_t index)
  : m_str(&s),
    m_index(index)
{}

utf8_string_const_iterator& utf8_string_const_iterator::operator++(){
  m_index++;
  return *this;
}

bool utf8_string_const_iterator::operator==(const utf8_string_const_iterator&
  other) const
{
  return m_str == other.m_str && m_index == other.m_index;
}

bool utf8_string_const_iterator::operator!=(const utf8_string_const_iterator& other) const
{
  return !operator==(other);
}

utf8_string_const_iterator& utf8_string_const_iterator::operator=(const utf8_string_const_iterator& other){
  m_str = other.m_str;
  m_index = other.m_index;
  return *this;
}

utf8_char utf8_string_const_iterator::operator*() const{
  return (*m_str)[m_index];
}

} // namespace
