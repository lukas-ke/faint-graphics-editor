// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/print-objects.hh"
#include "tests/test-util/text-bitmap.hh"
#include "bitmap/brush.hh"
#include "rendering/render-brush.hh"

void test_brush_overlay(){
  using namespace faint;
  AlphaMap overlay({1,1});

  init_brush_overlay(overlay, create_brush(IntSize(4,4),
      " XX "
      "XXXX"
      "XXXX"
      " XX ",
      {{' ', 0}, {'X', 1}}));

  // Verify that the overlay matches the brush we defined
  check(overlay,
    " XX "
    "XXXX"
    "XXXX"
    " XX ",
    {{' ', 0}, {'X', 1}});
  EQUAL(overlay.GetSize(), IntSize(4,4));

  // Set the same brush again to ensure the overlay is reset
  // rather than incremented.
  init_brush_overlay(overlay, create_brush(IntSize(4,4),
      // A circular brush
      " XX "
      "XXXX"
      "XXXX"
      " XX ",
      {{' ', 0}, {'X', 1}}));

  check(overlay,
    " XX "
    "XXXX"
    "XXXX"
    " XX ",
    {{' ', 0}, {'X', 1}});
  EQUAL(overlay.GetSize(), IntSize(4,4));

  // Set a brush with different dimensions
  init_brush_overlay(overlay, create_brush(IntSize(2,6),
      // A circular brush
      "XO"
      "O "
      "XO"
      "OX"
      " O"
      "OX",
      {{' ', 0}, {'X', 1}, {'O', 255}}));
  check(overlay,
      "XO"
      "O "
      "XO"
      "OX"
      " O"
      "OX",
    {{' ', 0}, {'X', 1}, {'O', 255}});
  EQUAL(overlay.GetSize(), IntSize(2,6));
}
