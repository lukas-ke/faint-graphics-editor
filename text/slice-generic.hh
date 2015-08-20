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

#ifndef FAINT_SLICE_HH
#define FAINT_SLICE_HH
#include <cassert>

namespace faint{ namespace generic{

size_t resolve_index(const size_t, int);

// Slice operations for getting substrings, similar to Pythons built
// in slice-syntax [a:b].
//
// Negative indexes are interpreted as length - index.
//
// Indexes past the end of the string are clamped to the end.
//
// If both indices are outside the string or the end index is <= the start index,
// an empty string is returned.

// Return the characters in the range [first, up_to)
template<typename StringType>
StringType slice(const StringType& s, int first, int up_to){
  size_t a = resolve_index(s.size(), first);
  size_t b = resolve_index(s.size(), up_to);

  if (a < b){
    auto s2 = s.substr(a, b - a);
    return s2;
  }
  return "";
}

// Return the characters from first to the end of the string
template<typename StringType>
StringType slice_from(const StringType& s, int first){
  if (s.empty()){
    return "";
  }

  size_t a = resolve_index(s.size(), first);
  return (a < s.size()) ?
    s.substr(a) : "";
}

// Return the characters up to the given index.
template<typename StringType>
StringType slice_up_to(const StringType& s, int up_to){
  if (s.empty()){
    return "";
  }
  const size_t b = resolve_index(s.size(), up_to);
  return s.substr(0, b);
}

// Return the character at pos as a string
template<typename StringType>
StringType char_at(const StringType& s, int pos){
  assert(!s.empty());
  if (pos < 0 && size_t(-pos) > s.size()){
    return "";
  }

  const size_t i = resolve_index(s.size(), pos);
  return (i < s.size()) ?
    s.substr(i, 1) : "";
}

}} // namespace

#endif
