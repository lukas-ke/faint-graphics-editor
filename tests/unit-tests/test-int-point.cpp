// -*- coding: us-ascii-unix -*-
#include <type_traits>
#include "test-sys/test.hh"
#include "tests/test-util/print-objects.hh"
#include "geo/int-point.hh"

void test_int_point(){
  using namespace faint;

  static_assert(std::is_literal_type<IntPoint>::value, "IntPoint not literal");

  IntPoint p0;
  EQUAL(p0, IntPoint(0,0));
  VERIFY(fully_positive(p0));
  EQUAL(max_coords(IntPoint(-1,-2), IntPoint(0,0)), IntPoint(0,0));
}
