// -*- coding: us-ascii-unix -*-
#include <algorithm> // std::sort
#include "test-sys/test.hh"
#include "tests/test-util/print-objects.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/color-counting.hh"
#include "geo/int-point.hh"
#include "geo/int-size.hh"

void test_color_counting(){
  using namespace faint;

  {
    // Four distinct colors.
    Bitmap bmp(IntSize(2,2), Color(255,255,255));
    EQUAL(count_colors(bmp), 1);

    put_pixel(bmp, {0,0}, color_red);
    put_pixel(bmp, {1,0}, color_green);
    put_pixel(bmp, {0,1}, color_blue);
    put_pixel(bmp, {1,1}, color_magenta);
    EQUAL(count_colors(bmp), 4);

    color_counts_t colorCounts;
    add_color_counts(bmp, colorCounts);
    EQUAL(colorCounts.size(), 4);

    EQUAL(colorCounts.at(color_red), 1);
    EQUAL(colorCounts.at(color_green), 1);
    EQUAL(colorCounts.at(color_blue), 1);
    EQUAL(colorCounts.at(color_magenta), 1);
  }
  // Fixme: Test unique_colors_rgb

}
