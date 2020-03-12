// -*- coding: us-ascii-unix -*-
#include <type_traits>
#include "test-sys/test.hh"
#include "geo/int-point.hh"
#include "util/distinct.hh"

void test_subtype(){
  using namespace faint;

  {
    class category_test_subtype_0;
    using TestPointA = Subtype<IntPoint, category_test_subtype_0, 0>;
    using TestPointB = Subtype<IntPoint, category_test_subtype_0, 1>;

    static_assert(std::is_assignable_v<IntPoint&, TestPointA>,
      "IntPoint should be assignable from SubType<IntPoint>");

    static_assert(!std::is_assignable_v<TestPointA&, IntPoint>,
      "SubType<IntPoint> should not be (implicitly) assignable from IntPoint");

    static_assert(!std::is_assignable_v<TestPointA&, TestPointB>,
      "Subtypes with different ids should not be assignable.");

    class category_test_subtype_1;
    using TestPointC = Subtype<IntPoint, category_test_subtype_1, 0>;

    static_assert(!std::is_assignable_v<TestPointA&, TestPointC>,
      "Subtypes with different categories should not be assigxnable.");

    const TestPointA tp(12, 24);
    VERIFY(tp == tp); // Equal to self

    const IntPoint ip(12, 24);
    VERIFY(tp == ip);

    IntPoint rawPoint(tp);
    VERIFY(rawPoint == tp && rawPoint == ip);

    VERIFY(tp.x == 12); // Subtype supports natural member access
  }

  {
    class category_test_primitive_subtype;
    using IntA = PrimitiveSubtype<int, category_test_primitive_subtype, 0>;
    using IntB = PrimitiveSubtype<int, category_test_primitive_subtype, 1>;

    static_assert(std::is_assignable_v<int&, IntA>,
      "T should be assignable from PrimitiveSubtype<T>");

    static_assert(!std::is_assignable_v<IntA&, int>,
      "PrimitiveSubType<T> should not not be (implicitly) assignable from T");

    static_assert(!std::is_assignable_v<IntA&, IntB>,
      "PrimitiveSubtype<T> with different ids should not be assignable.");

    {
      const int value = 10;
      IntA a(value);
      VERIFY(a == a);
      VERIFY(a <= a);
      // Operators available via type conversion
      VERIFY(a == value);
      VERIFY(a <= value);
      VERIFY(a < value + 1);
    }

  }
}
