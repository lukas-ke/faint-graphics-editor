// -*- coding: us-ascii-unix -*-
#include <limits>
#include "test-sys/test.hh"
#include "geo/measure.hh"
#include "geo/line.hh"
#include "geo/angle.hh"
#include "tests/test-util/print-objects.hh"

void test_measure(){
  using namespace faint;

  const auto eps = Epsilon(std::numeric_limits<double>::epsilon());
  NEAR(line_angle({{0,0},{1,0}}).Deg(), 0.0, eps);
  NEAR(line_angle({{0,0},{1,1}}).Deg(), 45.0, eps);
  NEAR(line_angle({{0,0},{0,1}}).Deg(), 90.0, eps);
  NEAR(line_angle({{0,0},{-1,1}}).Deg(), 135.0, eps);

  NEAR(line_angle({{0,0},{-1,0}}).Deg(), 180.0, eps);
  NEAR(line_angle({{0,0},{1,-1}}).Deg(), -45.0, eps);
  NEAR(line_angle({{0,0},{0,-1}}).Deg(), -90.0, eps);
  NEAR(line_angle({{0,0},{-1,-1}}).Deg(), -135.0, eps);
}
