// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/print-objects.hh"
#include "tests/test-util/text-bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/draw.hh"


void test_boundary_fill(){
  using namespace faint;

  const std::string s =
    "............."
    ".###########."
    ".#         #."
    ".# XXXXXXX #."
    ".# X     X #."
    ".# X XXX X #."
    ".# X X X X #."
    ".# X X X X #."
    ".# X   X X #."
    ".# XXXXX X #."
    ".#         #."
    ".###########."
    ".............";

  Bitmap bmp(create_bitmap({13,13}, s, {{'.', color_white},
                                 {'#', color_black},
                                 {'X', color_black},
                                 {' ', color_white}}));

  EQUAL(get_color(bmp, {0,0}), color_white);
  EQUAL(get_color(bmp, {1,1}), color_black);
  EQUAL(get_color(bmp, {12,12}), color_white);
  EQUAL(get_color(bmp, {11,11}), color_black);

  // Outside
  boundary_fill(bmp, {0,0}, Paint(color_red), color_black);
  FWD(check(bmp, s,  {{'.', color_red},
                     {'#', color_black},
                     {'X', color_black},
                     {' ', color_white}}));

  // Inside
  boundary_fill(bmp, {2,2}, Paint(color_blue), color_black);
  FWD(check(bmp, s,  {{'.', color_red},
                     {'#', color_black},
                     {'X', color_black},
                     {' ', color_blue}}));

  // No change when starting on boundary color
  boundary_fill(bmp, {1,1}, Paint(color_blue), color_black);
  FWD(check(bmp, s,  {{'.', color_red},
                     {'#', color_black},
                     {'X', color_black},
                     {' ', color_blue}}));

  // Fill the frame (#)
  boundary_fill(bmp, {1,1}, Paint(color_red), color_blue);
  FWD(check(bmp, s,  {{'.', color_red},
                      {'#', color_red},
                      {'X', color_black},
                      {' ', color_blue}}));

  // Fill all
  boundary_fill(bmp, {6,6}, Paint(color_green), color_magenta);
  FWD(check(bmp, s,  {{'.', color_green},
                      {'#', color_green},
                      {'X', color_green},
                      {' ', color_green}}));
}
