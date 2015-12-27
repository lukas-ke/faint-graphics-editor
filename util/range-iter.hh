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

#ifndef FAINT_RANGE_ITER_HH
#define FAINT_RANGE_ITER_HH
#include <iterator>
#include "geo/range.hh"

namespace faint{

class range_iter_t{
public:
  using value_type = int;
  using difference_type = int;
  using pointer = int*;
  using reference = int&;
  using iterator_category = std::random_access_iterator_tag;

  explicit range_iter_t(int current)
    : m_current(current)
  {}

  int operator*() const{
    return m_current;
  }

  void operator++(){
    m_current++;
  }

  range_iter_t& operator++(int){
    m_current++;
    return *this;
  }

  bool operator==(const range_iter_t& rhs) const{
    return m_current == rhs.m_current;
  }

  bool operator!=(const range_iter_t& rhs) const{
    return m_current != rhs.m_current;
  }

  int operator-(const range_iter_t& rhs) const{
    return m_current - rhs.m_current;
  }

private:
  int m_current;
};

inline auto begin(const ClosedIntRange& r){
  return range_iter_t(r.Lower());
}

inline auto end(const ClosedIntRange& r){
  return range_iter_t(r.Upper() + 1);
}

} // namespace

#endif
