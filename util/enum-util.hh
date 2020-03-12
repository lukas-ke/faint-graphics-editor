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

#ifndef FAINT_ENUM_UTIL_HH
#define FAINT_ENUM_UTIL_HH
#include "util/distinct.hh"
#include "util/optional.hh"

namespace faint{

template<typename ENUM_TYPE>
auto enum_min_value(){
  using UT = std::underlying_type_t<ENUM_TYPE>;
  return static_cast<UT>(ENUM_TYPE::MIN_VALUE);
}

template<typename ENUM_TYPE>
auto enum_max_value(){
  using UT = std::underlying_type_t<ENUM_TYPE>;
  return static_cast<UT>(ENUM_TYPE::MAX_VALUE);
}

template<typename ENUM_TYPE>
bool within_enum(std::underlying_type_t<ENUM_TYPE> v){
  return enum_min_value<ENUM_TYPE>() <= v && v <= enum_max_value<ENUM_TYPE>();
}

// Convert the value to the enum entry, or to the given default entry
// if the value is outside the enum.
// The enum must have contiguous values, and MIN_VALUE and MAX_VALUE
// entries defining the valid range of values.
template<typename ENUM_TYPE>
ENUM_TYPE to_enum(std::underlying_type_t<ENUM_TYPE> v,
  Default<ENUM_TYPE> defaultValue)
{
  return within_enum<ENUM_TYPE>(v) ?
    static_cast<ENUM_TYPE>(v) : defaultValue.Get();
}

// Returns the value converted to the enum, or an empty Optional.
template<typename ENUM_TYPE>
Optional<ENUM_TYPE> to_enum(std::underlying_type_t<ENUM_TYPE> v){
  if (within_enum<ENUM_TYPE>(v)){
    return {static_cast<ENUM_TYPE>(v)};
  }
  else{
    return {};
  }
}

} // namespace

#endif
