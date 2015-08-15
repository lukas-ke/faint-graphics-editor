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

#include <algorithm> // min, max
#include "slice.hh"

namespace faint{

static size_t resolve_index(const size_t len, int i){
  if (i < 0){
    size_t fromEnd = static_cast<size_t>(-i);
    return fromEnd > len ? 0 : len - fromEnd;
  }
  else{
    return std::min(len, static_cast<size_t>(i));
  }
}

utf8_string slice(const utf8_string& s, int first, int up_to){
  size_t a = resolve_index(s.size(), first);
  size_t b = resolve_index(s.size(), up_to);

  if (a < b){
    return s.substr(a, b - a);
  }
  return "";
}

utf8_string slice_from(const utf8_string& s, int from){
  size_t a = resolve_index(s.size(), from);
  return (a < s.size()) ?
    s.substr(a) : "";
}

utf8_string slice_up_to(const utf8_string& s, int up_to){
  const size_t b = resolve_index(s.size(), up_to);
  return s.substr(0, b);
}

} // namespace
