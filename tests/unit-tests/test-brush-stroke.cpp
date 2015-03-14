// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/text-bitmap.hh"
#include "bitmap/alpha-map.hh"
#include "bitmap/brush.hh"
#include "geo/int-point.hh"

void test_brush_stroke(){
  using namespace faint;
  const auto valueMap = alphamap_value_map({{'.', 0u}, {'X', 255u}});

  {
    // Single pixel brush
    AlphaMap map(IntSize(10,5));

    Brush b1(create_brush({1,1},
      "X",
      {{'X', 255u}}));
    stroke(map, UpperLeft({1,1}), UpperLeft({8,1}), b1);
    check(map,
      ".........."
      ".XXXXXXXX."
      ".........."
      ".........."
      "..........",
      valueMap);
  }

  {
    // Two pixel tall brush

    AlphaMap map(IntSize(10,5));
    Brush b2(create_brush({1,2},
      "X"
      "X",
      valueMap));

    stroke(map, UpperLeft({1,2}), UpperLeft({8,2}), b2);
    check(map,
      ".........."
      ".........."
      ".XXXXXXXX."
      ".XXXXXXXX."
      "..........",
      valueMap);
  }
}
