// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/print-objects.hh"
#include <type_traits>
#include "geo/angle.hh"

namespace test{
  template<> double to_double(const faint::Angle& a){
    return a.Rad();
  }
}

void test_angle(){
  using namespace faint;

  static_assert(std::is_literal_type<Angle>::value,
    "Angle not considered literal");

  const auto eps = 0.0001_eps;
  EQUAL(Angle::Zero(), Angle::Rad(0));
  EQUAL(Angle::Zero(), Angle::Deg(0));
  NEAR(sin(90_deg), 1.0, eps);
  NEAR(sin(0_deg), 0.0, eps);
  NEAR(cos(0_deg), 1.0, eps);
  NEAR(cos(90_deg), 0.0, eps);
  NEAR(cos(pi), -1.0, eps);
  NEAR(sin(pi), 0.0, eps);

  NEAR(faint::atan2(1.0, 0.0), pi / 2, eps);
  NEAR(faint::atan2(-1.0, 0.0), -pi / 2, eps);
  NEAR(faint::atan2(0.0, 1.0), 0_deg, eps);
  NEAR(faint::atan2(0.0, -1.0), pi, eps);

  NEAR(pi.Deg(), 180.0, eps);
  NEAR(-pi.Deg(), -180.0, eps);
  NEAR((-pi / 2).Rad(), (-(pi / 2)).Rad(), eps);
  NEAR(abs(-pi), pi, eps);

  NEAR(normalized(3 * pi), pi, eps);
  NEAR(tau, 2 * pi, eps);
  NEAR(pi * 2, pi + pi, eps);
  VERIFY(Angle::Zero() < pi);
  VERIFY(Angle::Zero() <= pi);
  VERIFY(pi > 0_deg);
  VERIFY(pi >= 0_deg);
  VERIFY(!(pi > pi));

  VERIFY(pi <= pi);
  VERIFY(pi >= pi);
  VERIFY(pi == pi);
  NEAR(abs(-pi), pi, eps);
  VERIFY(rather_zero(Angle::Zero()));
  NEAR(-pi, pi * -1, eps);
}
