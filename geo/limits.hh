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

#include <limits>

namespace faint{

// Various "can_represent"-functions which return true if the
// specified type can hold the passed in value (presumably of a different type).
//
// Usage: can_represent<int>(someValue)
//
// Overloaded via enable_if to avoid invalid signed/unsigned comparisons.
template <typename DST, typename SRC>
static typename std::enable_if<std::is_signed<DST>::value &&
std::is_signed<SRC>::value, bool>::type
can_represent(SRC value) {
  return !(value < std::numeric_limits<DST>::min() || value > std::numeric_limits<DST>::max());
}

template <typename DST, typename SRC>
static typename std::enable_if<std::is_signed<DST>::value &&
std::is_unsigned<SRC>::value, bool>::type
can_represent(SRC value) {
  return !(value > static_cast<typename std::make_unsigned<DST>::type>(std::numeric_limits<DST>::max()));
}

template <typename DST, typename SRC>
static typename std::enable_if<std::is_unsigned<DST>::value &&
std::is_signed<SRC>::value, bool>::type
can_represent(SRC value) {
  return !(value < 0 || static_cast<typename std::make_unsigned<SRC>::type>(value) > std::numeric_limits<DST>::max());
}

template <typename DST, typename SRC>
static typename std::enable_if<std::is_unsigned<DST>::value &&
std::is_unsigned<SRC>::value, bool>::type
can_represent(SRC value) {
  return !(value > std::numeric_limits<DST>::max());
}

template<typename DST, typename SRC>
DST asserting_static_cast(SRC v){
  assert(can_represent<DST>(v));
  return static_cast<DST>(v);
}

} // namespace
