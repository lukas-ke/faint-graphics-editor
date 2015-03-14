// -*- coding: us-ascii-unix -*-
#include <limits>
#include "tests/test-util/print-objects.hh"
#include "test-sys/test.hh"
#include "geo/int-size.hh"

void test_int_size(){
  using namespace faint;

  {
    // area
    EQUAL(area(IntSize(1, 1)), 1);
    EQUAL(area(IntSize(10, 2)), 20);
  }

  {
    // area_less
    auto maxInt = std::numeric_limits<int>::max();
    NOT(area_less(IntSize(10, 10), 100));
    VERIFY(area_less(IntSize(10, 10), 101));

    NOT(area_less(IntSize(maxInt, maxInt), maxInt));
    NOT(area_less(IntSize(maxInt, 1), maxInt));
    VERIFY(area_less(IntSize(maxInt - 1, 1), maxInt));
  }

  {
    EQUAL(min_coords(IntSize(1, 2), IntSize(5, 0)), IntSize(1, 0));
    EQUAL(max_coords(IntSize(1, 2), IntSize(5, 0)), IntSize(5, 2));
  }
}
