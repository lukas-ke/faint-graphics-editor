// -*- coding: us-ascii-unix -*-
#include <limits>
#include "test-sys/test.hh"
#include "geo/size.hh"

void test_size(){
  using namespace faint;

  {
    // area_less
    const auto maxCoord = std::numeric_limits<coord>::max();
    NOT(area_less(Size(10, 10), 100));
    VERIFY(area_less(Size(10, 10), 100.1));

    NOT(area_less(Size(maxCoord, maxCoord), maxCoord));
    NOT(area_less(Size(maxCoord, 1), maxCoord));
    VERIFY(area_less(Size(maxCoord, 0.99), maxCoord));
    NOT(area_less(Size(maxCoord, 1.0), maxCoord));

  }
}
