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

#ifndef FAINT_POINT_GENERATOR_HH
#define FAINT_POINT_GENERATOR_HH
#include <iterator>
#include "geo/int-point.hh"

namespace faint{

class point_iter{
public:
  point_iter(IntPoint p0, int w, int num)
    : m_num(num),
      m_p0(p0),
      m_w(w)
  {}

  point_iter()
    : m_num(-1),
      m_w(0)
  {}

  IntPoint operator*() const{
    int xd = m_num % m_w;
    return IntPoint(m_p0.x + xd,
      m_p0.y + (m_num / m_w));
  }

  void operator++(){
    ++m_num;
  }

  bool operator!=(const point_iter& rhs) const{
    return m_num != rhs.m_num;
  }

  using value_type = IntPoint;
  using difference_type = int;
  using pointer = IntPoint*;
  using reference = IntPoint&;
  using iterator_category = std::input_iterator_tag;
  point_iter& operator=(const point_iter) = delete;
private:
  int m_num;
  const IntPoint m_p0;
  const int m_w;
};

class point_range{
  // Allows range-for-loops between two integer points
public:
  point_range(const IntPoint& p0, const IntPoint& p1)
    : it_begin(p0, p1.x - p0.x + 1, 0),
      it_end(p0, p1.x - p0.x + 1, (p1.x - p0.x + 1) * (p1.y - p0.y + 1))
  {}
  const point_iter it_begin;
  const point_iter it_end;
};

inline point_iter begin(const point_range& r){
  return r.it_begin;
}

inline point_iter end(const point_range& r){
  return r.it_end;
}

} // namespace

#endif
