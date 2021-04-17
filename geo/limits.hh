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

#ifndef FAINT_LIMITS_HH
#define FAINT_LIMITS_HH

#include <cassert>
#include <limits>

namespace faint{

// Returns true if the specified type can hold the passed in value (of
// a different type).
//
// Usage: can_represent<int>(someValue)
template <typename DST, typename SRC>
bool can_represent(SRC value){
  // Separate implementations to avoid invalid signed/unsigned
  // comparisons
  if constexpr (std::is_signed_v<DST> && std::is_signed_v<SRC>){
    return !(value < std::numeric_limits<DST>::min() ||
             value > std::numeric_limits<DST>::max());
  }
  else if constexpr (std::is_signed_v<DST> && std::is_unsigned_v<SRC>){
    return !(value > static_cast<std::make_unsigned_t<DST>>(std::numeric_limits<DST>::max()));
  }
  else if constexpr (std::is_unsigned_v<DST> && std::is_signed_v<SRC>){
    return !(value < 0 || static_cast<std::make_unsigned_t<SRC>>(value) > std::numeric_limits<DST>::max());
  }
  else if constexpr (std::is_unsigned_v<DST> && std::is_unsigned_v<SRC>) {
    return !(value > std::numeric_limits<DST>::max());
  }
  else {
    static_assert(false);
  }
}

template<typename DST, typename SRC>
DST asserting_static_cast(SRC v){
  assert(can_represent<DST>(v));
  return static_cast<DST>(v);
}

// Wrapper which converts the held value to other types using
// asserting_static_cast
template<typename T>
class Convertible{
public:
  explicit Convertible(T v) :
    m_v(v){}

  template<typename DST>
  operator DST() const{
    return asserting_static_cast<DST>(m_v);
  }

private:
  T m_v;
};

template<typename T, typename T2>
T& operator+=(T& lhs, const Convertible<T2>& rhs){
  lhs += static_cast<T>(rhs);
  return lhs;
}

// Wrap the value for conversion to a different type, e.g.
// int v = convert(someValue);
//
// Note: Asserts that the target-type can_represent the value.
template<typename T>
auto convert(T v){
  return Convertible<T>(v);
}

} // namespace

#endif
