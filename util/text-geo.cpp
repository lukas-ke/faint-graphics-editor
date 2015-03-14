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

#include <cassert>
#include "util/text-geo.hh"
#include "text/text-buffer.hh"
#include "geo/size.hh"
#include "geo/tri.hh"

namespace faint{

Align::Align(HorizontalAlign h, VerticalAlign v)
  : horizontal(h),
    vertical(v)
{}

size_t caret_from_extents(const std::vector<int>& extents,
  const Point& pos, coord x0)
{
  //<../doc/caret-pos-delta.png>

  assert(!extents.empty());
  for (size_t left = 0; left != extents.size() - 1; left++){
    const size_t right = left + 1;
    coord d0 = pos.x - (x0 + extents[left]);
    coord d1 = pos.x - (x0 + extents[right]);
    if (d1 < 0){
      return (std::abs(d0) < std::abs(d1)) ?
        left : right;
    }
  }
  return extents.size() - 1;
}

int char_from_extents(const std::vector<int>& extents,
  const Point& pointerPos, coord x0)
{
  assert(extents.front() == 0);

  const int numChars = resigned(extents.size()) - 1;
  for (int ch = 1; ch <= numChars; ch++){
    if (pointerPos.x < (x0 + extents[to_size_t(ch)])){
      return ch - 1;
    }
  }

  // Outside to the right, return the last character index.
  return numChars - 1;
}

Tri aligned(const Tri& tri, HorizontalAlign align, coord part, coord whole){
  // <../doc/text-centering.png>

  if (align == HorizontalAlign::CENTER){
    return offset_aligned(tri, (whole - part) / 2, 0.0);
  }
  else if (align == HorizontalAlign::RIGHT){
    return offset_aligned(tri, (whole - part), 0.0);
  }
  else{
    return tri;
  }
}

TextPos index_to_row_column(const text_lines_t& lines, size_t caret){
  TextPos pos;
  pos.row = pos.col = 0;
  size_t chars = 0;

  for (size_t row = 0; row != lines.size(); row++){
    pos.row = row;
    const utf8_string& line = lines[row].text;
    size_t numChars = line.size();
    chars += numChars;
    if (chars > caret) {
      pos.col = numChars - (chars - caret);
      break;
    }
    pos.col = numChars - 1;
  }
  return pos;
}

static coord get_y_offset(const Tri& tri, coord rowHeight, int numRows,
  VerticalAlign align)
{
  switch(align){
  case VerticalAlign::TOP:
    return 0.0;

  case VerticalAlign::MIDDLE:
    return (tri.Height() - rowHeight * numRows) / 2.0;

  case VerticalAlign::BOTTOM:
    return (tri.Height() - rowHeight * numRows);
  }

  assert(false);
  return 0.0;
}

std::vector<Tri> text_selection_region(const TextInfo& info,
  const Tri& tri,
  const text_lines_t& lines,
  const CaretRange& selection,
  const Align& align)
{
  if (selection.Empty()){
    return {};
  }

  // Find the start and end positions
  TextPos s0 = index_to_row_column(lines, selection.from);
  TextPos s1 = index_to_row_column(lines, selection.to);

  Size size0 = info.TextSize(lines[s0.row].text.substr(0, s0.col));
  Size size1 = info.TextSize(lines[s1.row].text.substr(0, s1.col));

  std::vector<Tri> rectangles;
  const coord rowHeight = info.ComputeRowHeight();
  for (size_t row = s0.row; row <= s1.row; row++){
    coord dx = static_cast<coord>(row) * rowHeight * sin(-tri.GetAngle());
    coord dy = static_cast<coord>(row) * rowHeight * cos(tri.GetAngle());
    Size rowSize = info.TextSize(lines[row].text);

    // Start of the row
    Tri t2 = translated(tri, dx, dy);

    // End of the selection
    coord right_offset = row == s1.row ? size1.w : rowSize.w;
    Tri t3 = offset_aligned(t2, right_offset + 1.0, 0.0);

    if (row == s0.row){
      // Offset the selection start
      t2 = offset_aligned(t2, size0.w, 0.0);
    }

    Tri rowTri(aligned(Tri(t2.P0(), t3.P0(), rowHeight),
      align.horizontal, lines[row].width, tri.Width()));
    rectangles.push_back(rowTri);
  }
  return rectangles;
}


std::vector<Tri> text_line_regions(const TextInfo& info,
  const Tri& tri,
  const text_lines_t& lines,
  const Align& align)
{
  std::vector<Tri> tris;

  coord rowHeight = info.ComputeRowHeight();
  coord yOffset = get_y_offset(tri, rowHeight, resigned(lines.size()),
    align.vertical);
  const auto angle(tri.GetAngle());
  const auto sinAngle(sin(-angle));
  const auto cosAngle(cos(angle));

  for (size_t row = 0; row != lines.size(); row++){
    const auto& line = lines[row];

    coord dx = (static_cast<coord>(row) * rowHeight + yOffset) * sinAngle;
    coord dy = (static_cast<coord>(row) * rowHeight + yOffset) * cosAngle;
    tris.emplace_back(aligned(translated(tri, dx, dy),
      align.horizontal, line.width, tri.Width()));
  }
  return tris;

}

Size text_extents(const TextInfo& info, const text_lines_t& lines){
  coord width = 0;
  for (size_t i = 0; i!= lines.size(); i++){
    width = std::max(width, info.GetWidth(lines[i].text));
  }

  return Size(width, info.ComputeRowHeight() * static_cast<coord>(lines.size())); // Fixme: Check cast
}

} // namespace
