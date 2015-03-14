// -*- coding: us-ascii-unix -*-
#include <type_traits>
#include "test-sys/test.hh"
#include "geo/int-point.hh"
#include "util/distinct.hh"

void test_subtype(){
  using namespace faint;

  class category_test_subtype;
  using TestPointA = Subtype<IntPoint, category_test_subtype, 0>;
  using TestPointB = Subtype<IntPoint, category_test_subtype, 1>;

  static_assert(std::is_assignable<IntPoint&, TestPointA>::value,
    "IntPoint should be assignable from SubType<IntPoint>");

  static_assert(!std::is_assignable<TestPointA&, IntPoint>::value,
    "SubType<IntPoint> should not be (implicitly) assignable from IntPoint");

  static_assert(!std::is_assignable<TestPointA&, TestPointB>::value,
    "Subtypes with different ids should not be assignable.");

  {
    TestPointA tp(12, 24);
    IntPoint ip(12, 24);
    VERIFY(tp == ip);
    IntPoint rawPoint(tp);
    VERIFY(rawPoint == tp && rawPoint == ip);
  }
  TestPointA a(12,11);
  TestPointB b(11,12);
  VERIFY(a == a);
}
