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

class IntSize;

class TextInfo{
  // Interface for retrieving info about the pixel size of a string in
  // a given context.
public:
  virtual ~TextInfo() = default;
  virtual int GetWidth(const utf8_string&) const = 0;
  virtual int ComputeRowHeight() const = 0;
  virtual IntSize TextSize(const utf8_string&) const = 0;
};

class TextLine{
  // A line of text, presumably reformatted by splitting to fit within
  // a certain width.
public:
  // Text line broken by reflowing at a space
  static TextLine SoftBreak(coord, const utf8_string&);

  // Text line split at a hard endline character
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
