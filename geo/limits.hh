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

template<typename T1, typename T2>
bool can_represent(T2 v){
  return
    std::numeric_limits<T1>::min() <= v && v <= std::numeric_limits<T1>::max();
}

template<typename DST, typename SRC>
DST asserting_static_cast(SRC v){
  assert(can_represent<DST>(v));
  return static_cast<DST>(v);
}

} // namespace
