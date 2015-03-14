// -*- coding: us-ascii-unix -*-
// Copyright 2012 Lukas Kemmer
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

#ifndef FAINT_CONTAINER_UTIL_HH
#define FAINT_CONTAINER_UTIL_HH
#include <string>
#include <vector>

namespace faint{

template<typename T>
const T& get_item(const std::vector<T>& v, int pos){
  if (pos >= 0){
    return v.at(to_size_t(pos));
  }
  else{
    return v.at(to_size_t(resigned(v.size()) + pos));
  }
}

template<typename T>
std::pair<T,T> last_two(const std::vector<T>& v){
  return std::make_pair(get_item(v,-2), get_item(v, -1));
}

} // namespace

#endif
