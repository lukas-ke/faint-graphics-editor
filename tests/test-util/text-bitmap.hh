// -*- coding: us-ascii-unix -*-
#ifndef FAINT_TEST_TEXT_BITMAP_HH
#define FAINT_TEST_TEXT_BITMAP_HH

#include <map>
#include <string>
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "geo/int-size.hh"

namespace faint{
class AlphaMap;
class Brush;

using bitmap_value_map = std::map<char, Color>;

// Functions for instantiating Bitmaps, Brushes and AlphaMaps from a
// string description (laid out in 2d) and a map of characters to
// colors (or values in the case of brush and alpha-map).
//
// Example:
//   auto blackCircleBmp = create_bitmap({4, 4},
//    " .. "
//    "...."
//    "...."
//    " .. ",
//    {{'.', color_black()},
//     {' ', color_white()}});

Bitmap create_bitmap(const IntSize&,
  const std::string&,
  const bitmap_value_map& charToColor);

void check(const Bitmap&,
  const std::string&,
  const bitmap_value_map&);

using brush_value_map = std::map<char, uchar>;

Brush create_brush(const IntSize&,
  const std::string&,
  const brush_value_map&);

using alphamap_value_map = std::map<char, uchar>;

void check(const AlphaMap&,
  const std::string&,
  const alphamap_value_map&);

} // namespace

#endif
