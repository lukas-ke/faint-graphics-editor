// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "bitmap/gradient.hh"

void test_linear_gradient(){
  using namespace faint;

  {
    // Same all
    auto lg1 = LinearGradient(Angle::Rad(2.0),
      {{Color(255,0,255), 0.0}, {Color(255,0,0), 1.0}});
    auto lg2 = LinearGradient(Angle::Rad(2.0),
      {{Color(255,0,255), 0.0}, {Color(255,0,0), 1.0}});
    VERIFY(lg1 == lg2);
    VERIFY(!(lg1 < lg2));
    VERIFY(!(lg2 > lg1));
    VERIFY(!(lg2 < lg1));
    VERIFY(!(lg1 > lg2));
    VERIFY(!(lg1 != lg2));
  }

  {
    // Different angle
    auto lg1 = LinearGradient(Angle::Rad(2.0),
      {{Color(255,0,255), 0.0}, {Color(255,0,0), 1.0}});
    auto lg2 = LinearGradient(Angle::Rad(3.0),
      {{Color(255,0,255), 0.0}, {Color(255,0,0), 1.0}});
    VERIFY(lg1 < lg2);
    VERIFY(lg2 > lg1);
    VERIFY(!(lg2 < lg1));
    VERIFY(!(lg1 > lg2));
    VERIFY(lg1 != lg2);
    VERIFY(!(lg1 == lg2));
  }

  {
    // Different stop count
    auto lg1 = LinearGradient(Angle::Rad(3.0),
      {{Color(255,0,255), 0.0}});
    auto lg2 = LinearGradient(Angle::Rad(3.0),
      {{Color(255,0,255), 0.0}, {Color(255,0,0), 1.0}});
    VERIFY(lg1 < lg2);
    VERIFY(lg2 > lg1);
    VERIFY(!(lg2 < lg1));
    VERIFY(!(lg1 > lg2));
    VERIFY(lg1 != lg2);
    VERIFY(!(lg1 == lg2));
  }

  {
    // Different color
    auto lg1 = LinearGradient(Angle::Rad(3.0),
      {{Color(255,0,254), 0.0}, {Color(255,0,0), 1.0}});
    auto lg2 = LinearGradient(Angle::Rad(3.0),
      {{Color(255,0,255), 0.0}, {Color(255,0,0), 1.0}});
    VERIFY(lg1 < lg2);
    VERIFY(lg2 > lg1);
    VERIFY(!(lg2 < lg1));
    VERIFY(!(lg1 > lg2));
    VERIFY(lg1 != lg2);
    VERIFY(!(lg1 == lg2));
  }

  {
    // Different offset
    auto lg1 = LinearGradient(Angle::Rad(3.0),
      {{Color(255,0,255), 0.0}, {Color(255,0,0), 0.5}});
    auto lg2 = LinearGradient(Angle::Rad(3.0),
      {{Color(255,0,255), 0.0}, {Color(255,0,0), 1.0}});
    VERIFY(lg1 < lg2);
    VERIFY(lg2 > lg1);
    VERIFY(!(lg2 < lg1));
    VERIFY(!(lg1 > lg2));
    VERIFY(lg1 != lg2);
    VERIFY(!(lg1 == lg2));
  }
}
