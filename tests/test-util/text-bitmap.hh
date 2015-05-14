// -*- coding: us-ascii-unix -*-
#ifndef FAINT_TEST_TEXT_BITMAP_HH
#define FAINT_TEST_TEXT_BITMAP_HH

#include <map>
#include <string>
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/mask.hh"
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
//    {{'.', color_black},
//     {' ', color_white}});

Bitmap create_bitmap(const IntSize&,
  const std::string&,
  const bitmap_value_map& charToColor);

struct BitmapAndMask{
  BitmapAndMask(const Bitmap& bmp, const Mask& mask)
    : bitmap(bmp), mask(mask)
  {}
  Bitmap bitmap;
  Mask mask;
  BitmapAndMask() = delete;
};

// Creates a bitmap and mask.
// The maskDescription must have the same
// size as the bitmapDescription, and contain the mask-characters (see
// create_mask).
BitmapAndMask create_bitmap_and_mask(const IntSize&,
  const std::string& bitmapDescription,
  const bitmap_value_map& charToColor,
  const std::string& maskDescription);

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

// Creates a mask from the string, which should contain
//  ' ' for false
//  '#' for true
Mask create_mask(const IntSize&, const std::string&);

} // namespace

#endif
