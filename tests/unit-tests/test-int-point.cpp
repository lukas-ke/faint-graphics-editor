// -*- coding: us-ascii-unix -*-
#include <type_traits>
#include "test-sys/test.hh"
#include "tests/test-util/print-objects.hh"
#include "geo/int-point.hh"

void test_int_point(){
  using namespace faint;

  static_assert(std::is_literal_type<IntPoint>::value,
    "IntPoint not literal");

  #ifdef _MSC_VER // Fixme: Not supported by clang 3.5.0 with libstdc++
  static_assert(std::is_trivially_copyable<IntPoint>::value,
    "IntPoint not trivially copyable");
  #endif

  IntPoint p0;
  EQUAL(p0, IntPoint(0,0));
  VERIFY(fully_positive(p0));
  EQUAL(max_coords(IntPoint(-1,-2), IntPoint(0,0)), IntPoint(0,0));
}
