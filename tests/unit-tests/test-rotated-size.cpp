// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "geo/rotated-size.hh"
#include "tests/test-util/print-objects.hh"

void test_rotated_size(){
  using namespace faint;

  auto newSize = get_rotated_size(Angle::Deg(45), IntSize(20, 20));
  KNOWN_INEQUAL(newSize, IntSize(30, 30)); // Becomes 30, 29
}
