// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/draw.hh"
#include "tests/test-util/print-objects.hh"

void test_bitmap_blend() {
  using namespace faint;

  auto blended_colors = [](const faint::Color& b, const faint::Color& f){
    // Note: Merely a regression/characterization test, I am not sure
    // how the blending should work exactly, especially how it should
    // affect alpha.
    return faint::color_from_ints(
      (f.r * f.a + (b.r * (255 - f.a))) / 255,
      (f.g * f.a + (b.g * (255 - f.a))) / 255,
      (f.b * f.a + (b.b * (255 - f.a))) / 255,
      b.a);
  };

  const Color backgroundColor(50, 70, 45, 100);
  const Color overlayColor(40, 30, 100, 80);
  const auto expectedColor = blended_colors(backgroundColor, overlayColor);

  Bitmap background(IntSize(5,5), backgroundColor);
  Bitmap overlay(IntSize(10,10), overlayColor);

  blend(at_top_left(overlay), onto(background));
  EQUAL(get_color_raw(background, 0,0), expectedColor);
}
