// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/print-objects.hh"
#include "tests/test-util/text-bitmap.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/draw.hh"
#include "geo/int-rect.hh"
#include "geo/line.hh"

void test_draw(){
  using namespace faint;
  const Color white(color_white);
  const Color red(color_red);
  {
    Bitmap bmp(IntSize(100,100), white);

    draw_line(bmp, {{1,1},{11,1}}, solid_1px(red));

    // Check that line includes start and end point.
    EQUAL(get_color(bmp, {0,1}), white);
    EQUAL(get_color(bmp, {1,1}), red);
    EQUAL(get_color(bmp, {11,1}), red);
    EQUAL(get_color(bmp, {12,1}), white);
    EQUAL(get_color(bmp, {6,0}), white);
    EQUAL(get_color(bmp, {6,2}), white);

    // Same, vertical
    clear(bmp, white);
    draw_line(bmp, {{1,1},{1,11}}, solid_1px(red));
    EQUAL(get_color(bmp, {1,0}), white);
    EQUAL(get_color(bmp, {1,1}), red);
    EQUAL(get_color(bmp, {1,11}), red);
    EQUAL(get_color(bmp, {1,12}), white);
    EQUAL(get_color(bmp, {0,6}), white);
    EQUAL(get_color(bmp, {2,6}), white);

    // Reversed
    clear(bmp, white);
    draw_line(bmp, {{11,1},{1,1}}, solid_1px(red));
    EQUAL(get_color(bmp, {0,1}), white);
    EQUAL(get_color(bmp, {1,1}), red);
    EQUAL(get_color(bmp, {11,1}), red);
    EQUAL(get_color(bmp, {12,1}), white);
    EQUAL(get_color(bmp, {6,0}), white);
    EQUAL(get_color(bmp, {6,2}), white);

    // draw_rect
    clear(bmp, white);
    draw_rect(bmp, {IntPoint(1,1), IntPoint(11,6)}, solid_1px(red));
    EQUAL(get_color(bmp, {0,1}), white);
    EQUAL(get_color(bmp, {1,0}), white);
    EQUAL(get_color(bmp, {1,1}), red);
    EQUAL(get_color(bmp, {2,2}), white);
    EQUAL(get_color(bmp, {11,6}), red);
    EQUAL(get_color(bmp, {12,6}), white);
    EQUAL(get_color(bmp, {11,7}), white);

    // Sub-bitmap includes rectangle end points
    auto bmp2(subbitmap(bmp, {IntPoint(1,1),IntPoint(11,6)}));
    EQUAL(bmp2.GetSize(), IntSize(11, 6));
    EQUAL(get_color(bmp2, {0,0}), red);
    EQUAL(get_color(bmp2, {10,5}), red);
    EQUAL(get_color(bmp2, {1,1}), white);
    EQUAL(get_color(bmp2, {9,4}), white);

    // 90-degree rotation
    auto bmp3 = rotate_90cw(bmp2);
    EQUAL(bmp3.GetSize(), IntSize(6,11));
    EQUAL(get_color(bmp3, {0,0}), red);
    EQUAL(get_color(bmp3, {5,10}), red);
    EQUAL(get_color(bmp3, {1,1}), white);
    EQUAL(get_color(bmp3, {4,9}), white);
  }
}
