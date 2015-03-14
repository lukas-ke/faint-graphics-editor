// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "bitmap/brush.hh"
#include "geo/int-point.hh"
#include "geo/point.hh"
#include "rendering/render-brush.hh"

void test_brush_top_left(){
  using namespace faint;

  // 1x1, should always cover the hovered pixel.
  VERIFY(brush_top_left({0,0}, circle_brush(1)) == (IntPoint(0,0)));
  VERIFY(brush_top_left({0.9,0.9}, circle_brush(1)) == (IntPoint(0,0)));
  VERIFY(brush_top_left({-1,-2}, circle_brush(1)) == IntPoint(-1,-2));
  VERIFY(brush_top_left({-1.5,-2.5}, circle_brush(1)) == IntPoint(-1,-2));

  // 2x2, even sized, should be centered on the
  // pixel-grid-intersection closest to the mouse position.
  VERIFY(brush_top_left({0,0}, circle_brush(2)) == IntPoint(-1,-1));
  VERIFY(brush_top_left({0.6,0.6}, circle_brush(2)) == IntPoint(0,0));
  VERIFY(brush_top_left({-1,-2}, circle_brush(2)) == IntPoint(-2,-3));
  VERIFY(brush_top_left({1,1}, circle_brush(2)) == IntPoint(0,0));
  VERIFY(brush_top_left({1.6,1}, circle_brush(2)) == IntPoint(1,0));
  VERIFY(brush_top_left({1,1.6}, circle_brush(2)) == IntPoint(0,1));
  VERIFY(brush_top_left({2,2}, circle_brush(2)) == IntPoint(1,1));
  VERIFY(brush_top_left({2.6,2.6}, circle_brush(2)) == IntPoint(2,2));

  // 3x3, odd-sized, should always have its center pixel over the
  // hovered pixel.
  VERIFY(brush_top_left({0,0}, circle_brush(3)) == IntPoint(-2,-2));
  VERIFY(brush_top_left({0.1,0.1}, circle_brush(3)) == IntPoint(-1,-1));
  VERIFY(brush_top_left({2,2}, circle_brush(3)) == IntPoint(1,1));
  VERIFY(brush_top_left({2.9,2.9}, circle_brush(3)) == IntPoint(1,1));
}
