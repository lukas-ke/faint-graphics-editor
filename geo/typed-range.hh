// -*- coding: us-ascii-unix -*-
// Copyright 2017 Lukas Kemmer
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

#ifndef FAINT_TYPED_RANGE_HH
#define FAINT_TYPED_RANGE_HH
#include <algorithm>
#include "geo/limits.hh"

namespace faint{

// Typed range allowing conversions
// e.g.
//
// int x = 1000;
// TypedRange<uchar> r(0, 255);
// uchar x = r.Constrain(x) -> x = 255
//
// Not requiring a static_cast<uchar>, since the range held the type,
// and still allowed constraining a larger value.
template<typename T>
class TypedRange{
public:
  TypedRange(T lower, T upper)
    : m_lower(lower),
      m_upper(upper)
  {}

  template<typename T2>
  T Constrain(T2 v) const{
    if (v < m_lower){
      return m_lower;
    }
    if (m_upper < v){
      return m_upper;
    }
    return static_cast<T>(v);
  }

  template<typename T2>
  bool Has(T2 v) const{
    return m_lower <= v && v <= m_upper;
  }

private:
  const T m_lower;
  const T m_upper;
};

template<class RANGE_T, class T>
bool has_all(const RANGE_T& range, T v){
  return range.Has(v);
}

template<class RANGE_T, class A, class ...B>
bool has_all(const RANGE_T& range, A head, B... tail){
  return has_all(range, head) && has_all(range, tail...);
}

} // namespace

#endif
