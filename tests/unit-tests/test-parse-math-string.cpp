// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "util/parse-math-string.hh"

static void near(const faint::Optional<faint::coord>& opt, faint::coord key){
  using namespace faint;
  static const auto e = 0.000001_eps;
  opt.Visit([&](coord v){ NEAR(v, key, e);}, FAIL_IF_CALLED());
}

void test_parse_math_string(){
  using namespace faint;

  const coord dontCare = 2358978943.0;

  FWD(near(parse_math_string("5 + 5", dontCare), 10.0));
  FWD(near(parse_math_string("50 %", 10.0), 5.0));
  FWD(near(parse_math_string("5 + 5 * 2", dontCare), 15.0));
  FWD(near(parse_math_string("5 + 5 * 2 - 20.0", dontCare), -5.0));
  FWD(near(parse_math_string("5+ 5 *2 -20.0", dontCare), -5.0));

  // Invalid values
  VERIFY(parse_math_string("5+", dontCare).NotSet());
  KNOWN_ERROR(parse_math_string("%", dontCare).NotSet());
}
