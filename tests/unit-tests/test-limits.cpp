// -*- coding: us-ascii-unix -*-
#include <cstdint>
#include "test-sys/test.hh"
#include "geo/limits.hh"

void test_limits(){
  using namespace faint;
  { // can_represent
    VERIFY(can_represent<int32_t>(std::numeric_limits<int32_t>::min()));
    VERIFY(can_represent<int32_t>(std::numeric_limits<int32_t>::max()));
    VERIFY(can_represent<uint32_t>(10));
    VERIFY(can_represent<int32_t>(uint32_t(10)));
    NOT(can_represent<int16_t>(std::numeric_limits<int32_t>::max()));
    NOT(can_represent<uint32_t>(-1));
  }

  { // convert
    int32_t v2 = convert(uint16_t(12));
    EQUAL(v2, 12);
  }
}
