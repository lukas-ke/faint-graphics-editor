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

#ifndef FAINT_SPLIT_STRING_HH
#define FAINT_SPLIT_STRING_HH
#include "text/text-line.hh"
#include "util/optional.hh"

namespace faint{

using max_width_t = Optional<coord>;

// Split the string at line breaks, and within lines that are longer
// than max-width pixels (if specified).
text_lines_t split_string(const TextInfo&, const utf8_string&,
  const max_width_t&);

} // namespace

#endif
