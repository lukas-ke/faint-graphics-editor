// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
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

#ifndef FAINT_TEXT_LINE_HH
#define FAINT_TEXT_LINE_HH
#include <vector>
#include "geo/primitive.hh"
#include "text/utf8-string.hh"

namespace faint{
class Size;

class TextInfo{
public:
  virtual ~TextInfo() = default;
  virtual coord GetWidth(const utf8_string&) const = 0;
  virtual coord ComputeRowHeight() const = 0;
  virtual Size TextSize(const utf8_string&) const = 0;
};

class TextLine{
public:
  static TextLine SoftBreak(coord, const utf8_string&);
  static TextLine HardBreak(coord, const utf8_string&);
  bool hardBreak;
  utf8_string text;
  coord width;
private:
  TextLine(bool, coord, const utf8_string&);
};

using text_lines_t = std::vector<TextLine>;

} // namespace

#endif
