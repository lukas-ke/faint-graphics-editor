// -*- coding: us-ascii-unix -*-
#include <cstdint>
#include "test-sys/test.hh"
#include "geo/limits.hh"

void test_limits(){
  using namespace faint;
  VERIFY(can_represent<int32_t>(std::numeric_limits<int32_t>::min()));
  VERIFY(can_represent<int32_t>(std::numeric_limits<int32_t>::max()));
  VERIFY(can_represent<uint32_t>(10));
  VERIFY(can_represent<int32_t>(uint32_t(10)));
  NOT(can_represent<int16_t>(std::numeric_limits<int32_t>::max()));
  NOT(can_represent<uint32_t>(-1));
}
