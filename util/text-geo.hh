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

#ifndef FAINT_TEXT_GEO_HH
#define FAINT_TEXT_GEO_HH
#include <vector>
#include "geo/primitive.hh"
#include "text/text-line.hh"
#include "util/setting-id.hh"

namespace faint{

class CaretRange;
class Point;
class Tri;

// Find the closest caret position within the cumulative character
// extents in the vector.
//
// Returns a caret position in the range [0, extents.size() - 1],
// where 0 means the caret should be in front of the left-most character,
// size - 1 after the last character.
size_t caret_from_extents(const std::vector<int>& extents,
  const Point& pointerPos, coord extentOffset);

// Returns the zero-based index of the character the position falls on
// within the extents, or the min or max character index if outside
// to the left or right.
int char_from_extents(const std::vector<int>& extents,
  const Point& pointerPos, coord extentOffset);

// Return the tri offset for the specified alignment. 'whole' is the
// width of the text region, 'part'' is the width of the text to be
// aligned within.
Tri aligned(const Tri&, HorizontalAlign, coord part, coord whole);

class TextPos{
public:
 TextPos(size_t row=0, size_t col=0)
   : row(row),
     col(col)
 {}

  bool operator==(const TextPos& other) const{
    return other.row == row && other.col == col;
  }

  size_t row;
  size_t col;
};

// Transforms an index into the text to the line and column of the
// text split into lines.
TextPos index_to_row_column(const text_lines_t&, size_t index);

class Align{
public:
  Align(HorizontalAlign h, VerticalAlign v);
  HorizontalAlign horizontal;
  VerticalAlign vertical;
};

std::vector<Tri> text_selection_region(const TextInfo&,
  const Tri&,
  const text_lines_t&,
  const CaretRange&,
  const Align&);

std::vector<Tri> text_line_regions(const TextInfo&,
  const Tri&,
  const text_lines_t&,
  const Align&);

Size text_extents(const TextInfo&, const text_lines_t&);

} // namespace

#endif
