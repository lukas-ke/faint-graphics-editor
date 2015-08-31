// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "geo/rotation-adjustment.hh"
#include "tests/test-util/print-objects.hh"

void test_rotation_adjustment(){
  using namespace faint;
  using RA = RotationAdjustment;

  auto a = get_rotation_adjustment(Angle::Deg(45), IntSize(20, 20));
  KNOWN_INEQUAL(a.size, IntSize(30, 30)); // Becomes 30, 29
  EQUAL(a.offset, IntPoint(-15, 0)); // Fixme: What is this?
}
