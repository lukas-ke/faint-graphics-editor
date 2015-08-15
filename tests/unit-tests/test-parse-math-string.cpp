// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "util/parse-math-string.hh"

void test_parse_math_string(){
  using namespace faint;
  const auto e = 0.000001_eps;
  const coord dontCare = 2358978943.0;

  NEAR(parse_math_string("5 + 5", dontCare), 10.0, e);
  NEAR(parse_math_string("50 %", 10.0), 5.0, e);
  NEAR(parse_math_string("5 + 5 * 2", dontCare), 15.0, e);
  NEAR(parse_math_string("5 + 5 * 2 - 20.0", dontCare), -5.0, e);
  NEAR(parse_math_string("5+ 5 *2 -20.0", dontCare), -5.0, e);

  // Invalid values
  NEAR(parse_math_string("5+", dontCare), -1.0, e);
  KNOWN_ERROR(test_near(parse_math_string("%", dontCare), -1.0, e));
}
