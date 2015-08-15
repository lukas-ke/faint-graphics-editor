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
#include "text/utf8-string.hh"

namespace faint{

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
utf8_string slice(const utf8_string&, int first, int up_to);

// Return the characters from first to the end of the string
utf8_string slice_from(const utf8_string&, int first);

// Return the characters up to the given index.
utf8_string slice_up_to(const utf8_string&, int up_to);

} // namespace

#endif
