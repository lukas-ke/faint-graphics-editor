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

#include "util/exception.hh"

namespace faint{

Exception::Exception(utf8_string&& str) noexcept
  : m_str(std::move(str))
{}

Exception::Exception(const utf8_string& str)
  : m_str(str)
{}

Exception::Exception(const char* str)
  : m_str(str)
{}

const utf8_string& Exception::What() const noexcept{
  return m_str;
}

const char* Exception::what() const noexcept{
  return m_str.c_str();
}

} // namespace
