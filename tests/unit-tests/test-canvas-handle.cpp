// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/print-objects.hh"
#include "app/canvas-handle.hh"
#include "text/formatting.hh"

void test_canvas_handle(){
  using namespace faint;
  using HP = HandlePos;
  auto fmt = [](auto e){ return str_int(static_cast<int>(e)); };

  EQUALF(opposite_handle_pos(HP::TOP_LEFT), HP::BOTTOM_RIGHT, fmt);
  EQUALF(opposite_handle_pos(HP::TOP_SIDE), HP::BOTTOM_SIDE, fmt);
  EQUALF(opposite_handle_pos(HP::TOP_RIGHT), HP::BOTTOM_LEFT, fmt);
  EQUALF(opposite_handle_pos(HP::RIGHT_SIDE), HP::LEFT_SIDE, fmt);
  EQUALF(opposite_handle_pos(HP::BOTTOM_RIGHT), HP::TOP_LEFT, fmt);
  EQUALF(opposite_handle_pos(HP::BOTTOM_SIDE), HP::TOP_SIDE, fmt);
  EQUALF(opposite_handle_pos(HP::BOTTOM_LEFT), HP::TOP_RIGHT, fmt);
  EQUALF(opposite_handle_pos(HP::LEFT_SIDE), HP::RIGHT_SIDE, fmt);

  using HD = HandleDirection;
  EQUALF(handle_direction(HP::TOP_LEFT), HD::DIAGONAL, fmt);
  EQUALF(handle_direction(HP::TOP_SIDE), HD::UP_DOWN, fmt);
  EQUALF(handle_direction(HP::TOP_RIGHT), HD::DIAGONAL, fmt);
  EQUALF(handle_direction(HP::RIGHT_SIDE), HD::LEFT_RIGHT, fmt);
  EQUALF(handle_direction(HP::BOTTOM_RIGHT), HD::DIAGONAL, fmt);
  EQUALF(handle_direction(HP::BOTTOM_SIDE), HD::UP_DOWN, fmt);
  EQUALF(handle_direction(HP::BOTTOM_LEFT), HD::DIAGONAL, fmt);
  EQUALF(handle_direction(HP::LEFT_SIDE), HD::LEFT_RIGHT, fmt);

  const IntSize sz(100,70);
  const CanvasGeo g;
  // Top left handle, upper left corner
  VERIFY(canvas_handle_hit_test(IntPoint(6,7), sz, g).NotSet());
  VERIFY(canvas_handle_hit_test(IntPoint(7,6), sz, g).NotSet());
  VERIFY(canvas_handle_hit_test(IntPoint(7,7), sz, g).IsSet());

  // Top left handle, Lower right corner
  VERIFY(canvas_handle_hit_test(IntPoint(12,12), sz, g).IsSet());
  VERIFY(canvas_handle_hit_test(IntPoint(13,12), sz, g).NotSet());
  VERIFY(canvas_handle_hit_test(IntPoint(12,13), sz, g).NotSet());

  auto topLeft(canvas_handle_hit_test(IntPoint(10,10), sz, g));
  ASSERT(topLeft.IsSet());
  EQUALF(topLeft.Get().GetDirection(), HD::DIAGONAL, fmt);
  EQUAL(topLeft.Get().Opposite().GetRect(),
    IntRect(IntPoint(127,97), IntSize(6,6)));
}
