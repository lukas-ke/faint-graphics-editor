// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "text/text-expression-conversions.hh"
#include "util/optional.hh"

namespace faint{

const Conversions& get(const std::map<utf8_string, Conversions>& c, const utf8_string& unit)
{
  auto it = c.find(unit);
  VERIFY(it != end(c));
  return it->second;
}

}

void test_text_expression_conversions(){
  using namespace faint;
  auto eps = 0.001_eps;
  {
    const auto& lengths = length_conversions();
    NEAR(get(lengths, "mm")["mm"].Get(), 1.0, eps);
    NEAR(get(lengths, "mm")["cm"].Get(), 10.0, eps);
    NEAR(get(lengths, "cm")["mm"].Get(), 0.1, eps);
    NEAR(get(lengths, "dm")["mm"].Get(), 0.01, eps);
    NEAR(get(lengths, "m")["mm"].Get(), 0.001, eps);
  }

  {
    // const auto& areas = area_conversions();
    // NEAR(get(areas, "mm2")["mm2"].Get(), 1.0, eps);
    // NEAR(get(areas, "mm2")["cm2"].Get(), 100.0, eps);
    // NEAR(get(areas, "mm2")["dm2"].Get(), 10000.0, eps);
    // NEAR(get(areas, "mm2")["m2"].Get(), 1000000.0, eps);
  }

  // Todo: Extend
  // also
  // Unit("mm").per(Unit("cm")) == 10.0
  // Unit("cm").per(Unit("mm")) == 0.1
}
