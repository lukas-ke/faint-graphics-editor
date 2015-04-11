// -*- coding: us-ascii-unix -*-
#include <cmath>
#include <limits>
#include "test-sys/test.hh"
#include "geo/angle.hh"
#include "geo/line.hh"
#include "geo/measure.hh"
#include "tests/test-util/print-objects.hh"

void test_measure(){
  using namespace faint;
  const auto eps = Epsilon(std::numeric_limits<double>::epsilon());

  // line_angle_cw
  // <../../doc/line_angle_cw.png>
  NEAR(line_angle_cw({{0,0},{1,0}}).Deg(), 0.0, eps);
  NEAR(line_angle_cw({{0,0},{-1,0}}).Deg(), 180.0, eps);

  // Lower semi-circle
  NEAR(line_angle_cw({{0,0},{1,1}}).Deg(), 45.0, eps);
  NEAR(line_angle_cw({{0,0},{0,1}}).Deg(), 90.0, eps);
  NEAR(line_angle_cw({{0,0},{-1,1}}).Deg(), 135.0, eps);

  // Upper-semi-circle
  NEAR(line_angle_cw({{0,0},{1,-1}}).Deg(), -45.0, eps);
  NEAR(line_angle_cw({{0,0},{0,-1}}).Deg(), -90.0, eps);
  NEAR(line_angle_cw({{0,0},{-1,-1}}).Deg(), -135.0, eps);

  // angle360_ccw
  // <../../doc/angle360_ccw.png>
  NEAR(angle360_ccw({{0,0},{1,0}}).Deg(), 0.0, eps);
  NEAR(angle360_ccw({{0,0},{1,-1}}).Deg(), 45.0, eps);
  NEAR(angle360_ccw({{0,0},{0,-1}}).Deg(), 90.0, eps);
  NEAR(angle360_ccw({{0,0},{-1,-1}}).Deg(), 135.0, eps);
  NEAR(angle360_ccw({{0,0},{-1,0}}).Deg(), 180.0, eps);
  NEAR(angle360_ccw({{0,0},{-1,1}}).Deg(), 225.0, eps);
  NEAR(angle360_ccw({{0,0},{0,1}}).Deg(), 270.0, eps);
  NEAR(angle360_ccw({{0,0},{1,1}}).Deg(), 315.0, eps);

  // distance
  NEAR(distance(Point(0,0), Point(1,0)), 1.0, eps);
  NEAR(distance(Point(0,0), Point(1,1)), std::sqrt(2), eps);
  NEAR(distance(Point(0,0), Point(-1,1)), std::sqrt(2), eps);
  NEAR(distance(Point(0,0), Point(1,-1)), std::sqrt(2), eps);
  NEAR(distance(Point(0,0), Point(-1,-1)), std::sqrt(2), eps);

}
