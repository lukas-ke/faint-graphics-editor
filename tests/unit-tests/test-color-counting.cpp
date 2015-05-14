// -*- coding: us-ascii-unix -*-
#include <algorithm> // std::sort
#include "test-sys/test.hh"
#include "tests/test-util/print-objects.hh"
#include "tests/test-util/text-bitmap.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/color-counting.hh"
#include "geo/int-point.hh"
#include "geo/int-size.hh"

void test_color_counting(){
  using namespace faint;

  const auto colors = bitmap_value_map{
    {'r', color_red},
    {'g', color_green},
    {'b', color_blue},
    {'m', color_magenta}};

  {
    // add_color_counts, four distinct colors
    auto bmp = create_bitmap(IntSize(2,2),
      "rg"
      "bm",
      colors);
    EQUAL(count_colors(bmp), 4);

    color_counts_t colorCounts;
    add_color_counts(bmp, colorCounts);
    EQUAL(colorCounts.size(), 4);

    EQUAL(colorCounts.at(to_hash(color_red)), 1);
    EQUAL(colorCounts.at(to_hash(color_green)), 1);
    EQUAL(colorCounts.at(to_hash(color_blue)), 1);
    EQUAL(colorCounts.at(to_hash(color_magenta)), 1);
  }

  {
    // add_color_counts, repeated magenta
    auto bmp = create_bitmap(IntSize(2,2),
      "rm"
      "bm",
      colors);
    EQUAL(count_colors(bmp), 3);

    color_counts_t colorCounts;
    add_color_counts(bmp, colorCounts);
    EQUAL(colorCounts.size(), 3);

    EQUAL(colorCounts.at(to_hash(color_red)), 1);
    EQUAL(colorCounts.at(to_hash(color_blue)), 1);
    EQUAL(colorCounts.at(to_hash(color_magenta)), 2);
  }

  // Fixme: Test unique_colors_rgb
}
