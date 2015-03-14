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
#include "util/index.hh"

namespace faint{

Index::Index(size_t value)
  : m_value(resigned(value))
{}

int Index::Get() const{
  return m_value;
}

bool operator==(const Index& lhs, const Index& rhs){
  return lhs.Get() == rhs.Get();
}

bool operator==(const Index& lhs, const size_t& rhs){
  return to_size_t(lhs) == rhs;
}

Index& Index::operator=(const Index& rhs){
  m_value = rhs.Get();
  return *this;
}

bool operator<(const Index& lhs, const Index& rhs){
  return lhs.Get() < rhs.Get();
}

bool operator>(const Index& lhs, size_t rhs){
  return to_size_t(lhs.Get()) > rhs;
}

bool operator<=(const Index& lhs, const Index& rhs){
  return lhs.Get() <= rhs.Get();
}

bool operator!=(const Index& lhs, const Index& rhs){
  return lhs.Get() != rhs.Get();
}

bool operator!=(const Index& lhs, size_t rhs){
  return to_size_t(lhs.Get()) != rhs;
}

bool operator<(const Index& lhs, size_t rhs){
  return to_size_t(lhs.Get()) < rhs;
}

bool operator<=(const Index& index, size_t rhs){
  size_t lhs = to_size_t(index);
  return lhs <= rhs;
}

Index operator-(const Index& index, size_t rhs){
  size_t lhs = to_size_t(index.Get());
  assert(lhs >= rhs);
  return Index(resigned(lhs - rhs));
}

size_t to_size_t(const Index& index){
  return to_size_t(index.Get());
}

Index to_index(size_t v){
  return Index(resigned(v));
}

} // namespace
