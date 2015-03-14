// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include <algorithm> // std::min, std::max
#include "bitmap/color.hh"
#include "tests/test-util/print-objects.hh"

void test_color(){
  using namespace faint;

  { // Color and ColRGB classes
    Color c1(30, 40, 50, 60);
    EQUAL(c1.r, 30);
    EQUAL(c1.g, 40);
    EQUAL(c1.b, 50);
    EQUAL(c1.a, 60);

    Color c2(ColRGB(30, 40, 50), 60);
    EQUAL(c1, c2);
    VERIFY(!(c1 < c2));
    VERIFY(!(c2 < c1));

    EQUAL(color_from_ints(30, 40, 50, 60), c1);
    EQUAL(rgb_from_ints(30, 40, 50), strip_alpha(c1));
  }

  { // Color::operator<
    const Color c3(20, 30, 40, 50);
    const Color c4(50, 40, 30, 20);

    // Which color is considered larger is not interesting,
    // this is only to allow sorting, insertion into sets etc.
    Color smaller(std::min(c3, c4));
    Color larger(std::max(c3, c4));
    VERIFY(smaller < larger);
    VERIFY(!(larger < smaller));
    VERIFY(smaller != larger);
    VERIFY(larger != smaller);
    VERIFY(!(larger == smaller));
    VERIFY(!(smaller == larger));
  }

  // Miscellaneous functions
  EQUAL(ColRGB(10, 20, 30), strip_alpha(Color(10, 20, 30, 72)));
  EQUAL(color_from_ints(300, -20, -20, -20), Color(255, 0, 0, 0));
  EQUAL(grayscale_rgb(103), ColRGB(103, 103, 103));
  EQUAL(grayscale_rgba(103), Color(103, 103, 103, 255));

  EQUAL(blend_alpha(Color(100, 30, 40, 128), ColRGB(255,0,100)),
    ColRGB(177, 15, 69));

  EQUAL(invert(ColRGB(255, 0, 128)), ColRGB(0, 255, 127));

  // Opacity query functions
  VERIFY(opaque(Color(10,20,30,255)));
  VERIFY(!translucent(Color(10,20,30,255)));
  VERIFY(!opaque(Color(10,20,30,254)));
  VERIFY(translucent(Color(10,20,30,254)));
  VERIFY(valid_color(0, 255, 100, 255));
  VERIFY(!valid_color(0, 255, 100, 256));

  // Color math functions
  EQUAL(subtract(Color(10,20,30,109), Color(11, 10, 21,90)),
    Color(0, 10, 9, 19));
  EQUAL(add(Color(10,20,30,40), Color(250, 10, 21, 60)),
    Color(255, 30, 51, 100));
  EQUAL(sum_rgb(Color(100,20,50, 42)), 170);
  EQUAL(sum_rgb(Color(255,255,255, 42)), 765);
  EQUAL(sum_rgb(Color(0,0,0, 42)), 0);

  // Hashing
  EQUAL(color_from_hash(to_hash(Color(0, 0, 0, 0))),
    Color(0, 0, 0, 0));

  EQUAL(color_from_hash(to_hash(Color(255, 255, 255, 255))),
    Color(255, 255, 255, 255));

  EQUAL(color_from_hash(to_hash(Color(100, 170, 193, 42))),
    Color(100, 170, 193, 42));
}
