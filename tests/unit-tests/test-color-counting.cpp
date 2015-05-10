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

  {
    // Three colors, red duplicated
    Bitmap bmp(IntSize(2,2), Color(255,255,255));
    EQUAL(count_colors(bmp), 1);
    put_pixel(bmp, {0,0}, color_red);
    put_pixel(bmp, {1,0}, color_red);
    put_pixel(bmp, {0,1}, color_blue);
    put_pixel(bmp, {1,1}, color_magenta);
    EQUAL(count_colors(bmp), 3);

    color_counts_t colorCounts;
    add_color_counts(bmp, colorCounts);
    EQUAL(colorCounts.size(), 3);
    EQUAL(colorCounts.at(color_red), 2);
    EQUAL(colorCounts.at(color_blue), 1);
    EQUAL(colorCounts.at(color_magenta), 1);
  }

  {
    // Two colors, only differing by alpha
    Bitmap bmp(IntSize(2,1));
    put_pixel(bmp, {0,0}, Color(255,0,0,100));
    put_pixel(bmp, {1,0}, Color(255,0,0,200));
    EQUAL(count_colors(bmp), 2);

    color_counts_t colorCounts;
    add_color_counts(bmp, colorCounts);
    EQUAL(colorCounts.at(Color(255,0,0,100)), 1);
    EQUAL(colorCounts.at(Color(255,0,0,200)), 1);
  }

  {
    // get_unique_colors

    // Three colors, red duplicated
    Bitmap bmp(IntSize(2,2), Color(255,255,255));
    put_pixel(bmp, {0,0}, color_red);
    put_pixel(bmp, {1,0}, color_red);
    put_pixel(bmp, {0,1}, color_blue);
    put_pixel(bmp, {1,1}, color_magenta);

    auto key = std::vector<Color>({color_red, color_blue, color_magenta});
    std::sort(begin(key), end(key));
    EQUAL(get_unique_colors(bmp), key);
  }

  {
    // merged_fully_transparent, empty
    auto preferred = ColRGB(10,60,20);
    std::vector<Color> colors;
    std::vector<Color> key;
    EQUAL(merged_fully_transparent(colors, preferred), key);
  }

  {
    // merged_fully_transparent
    auto preferred = ColRGB(10,60,20);
    std::vector<Color> colors =
      {with_alpha(color_blue, 0),
       with_alpha(color_gray, 10),
       with_alpha(color_red, 0),
       with_alpha(color_magenta, 255)};
    std::vector<Color> key =
      {with_alpha(color_gray, 10),
       with_alpha(color_magenta, 255),
       with_alpha(preferred, 0)};
    EQUAL(merged_fully_transparent(colors, preferred), key);
  }

  {
    // merged_fully_transparent, no fully transparent
    auto preferred = ColRGB(10,60,20);

    std::vector<Color> colors =
      {with_alpha(color_blue, 40),
       with_alpha(color_gray, 1),
       with_alpha(color_red, 30),
       with_alpha(color_magenta, 255)};
    std::vector<Color> key =
      {with_alpha(color_blue, 40),
       with_alpha(color_gray, 1),
       with_alpha(color_red, 30),
       with_alpha(color_magenta, 255)};
    EQUAL(merged_fully_transparent(colors, preferred), key);
  }
}
