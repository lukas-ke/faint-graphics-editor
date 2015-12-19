// -*- coding: us-ascii-unix -*-
#include <algorithm>
#include <cmath>
#include <limits>
#include "test-sys/test.hh"
#include "geo/angle.hh"
#include "geo/int-rect.hh"
#include "geo/line.hh"
#include "geo/rect.hh"
#include "geo/measure.hh"
#include "tests/test-util/print-objects.hh"
#include "util/math-constants.hh"

void test_measure(){
  using namespace faint;
  const auto eps = test::Epsilon(std::numeric_limits<double>::epsilon());

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

  // distance
  NEAR(distance(Point(0,0), Point(1,0)), 1.0, eps);
  NEAR(distance(Point(0,0), Point(1,1)), std::sqrt(2), eps);
  NEAR(distance(Point(0,0), Point(-1,1)), std::sqrt(2), eps);
  NEAR(distance(Point(0,0), Point(1,-1)), std::sqrt(2), eps);
  NEAR(distance(Point(0,0), Point(-1,-1)), std::sqrt(2), eps);

  // Note: bezier-distance tested in bezier-distance.cpp

  // ellipse_perimeter
  NEAR(ellipse_perimeter(1.0, 1.0), math::tau, eps);
  NEAR(ellipse_perimeter(0.5, 0.5), math::pi, eps);
  NEAR(ellipse_perimeter(10.0, 20.0), 96.884, 0.001_eps);

  // mid_point
  EQUAL(mid_point({1.0, 1.0}, {7.0, 5.0}), Point(4, 3));
  EQUAL(mid_point({1.0, 1.0}, {-7.0, -5.0}), Point(-3, -2));

  // mid_points
  using pt_vec = std::vector<Point>;
  EQUAL(mid_points({}), pt_vec());
  EQUAL(mid_points({{1.0, 1.0}}), pt_vec());
  EQUAL(mid_points({{1.0, 1.0}, {7.0, 5.0}}), pt_vec({{4.0, 3.0}}));
  EQUAL(mid_points({{1.0, 1.0}, {7.0, 5.0}, {1.0, 1.0}, {-7.0, -5.0}}),
    pt_vec({{4.0, 3.0}, {4.0, 3.0}, {-3.0, -2.0}}));

  {
    // with_mid_points
    EQUAL(with_mid_points({}), pt_vec());

    // Unmodified for less than two points
    EQUAL(with_mid_points({{1.0,1.0}}), pt_vec({{1.0, 1.0}}));

    EQUAL(with_mid_points({{1.0, 1.0}, {2.0, 2.0}}),
      pt_vec({{1.0, 1.0}, {1.5, 1.5}, {2.0, 2.0}}));
  }

  {
    // with_mid_points_cyclic
    EQUAL(with_mid_points_cyclic({}), pt_vec());

    // Unmodified for less than two points
    EQUAL(with_mid_points_cyclic({{1.0,1.0}}), pt_vec({{1.0, 1.0}}));

    EQUAL(with_mid_points_cyclic({{1.0, 1.0}, {2.0, 2.0}}),
      pt_vec({{1.5, 1.5}, {1.0, 1.0}, {1.5, 1.5}, {2.0, 2.0}}));
  }

  {
    // Variadic bounding_rect (mixed)
    LineSegment l({3,7}, {11,10});
    Rect r{Point(9,2), Point(10,15)};
    const auto key = Rect(Point(3,2), Point(11,15));
    EQUAL(key, bounding_rect(l, r));
  }

  {
    std::vector<IntPoint> pts = {{1,3}, {10, 2}, {23, 2}};
    IntRect key{IntPoint(1,2), IntPoint(23,3)};
    std::sort(begin(pts), end(pts));
    int i = 0;
    do{
      EQUAL(bounding_rect(pts[0], pts[1], pts[2]), key);
      i++;
    } while (std::next_permutation(begin(pts), end(pts)));
    EQUAL(i,6);
  }

  {
    // Variadic bounding_rect, decimal points
    std::vector<Point> pts = {{1.2, 3.2}, {10, 2}, {23.4, 2}};
    Rect key{Point(1.2,2), Point(23.4,3.2)};
    std::sort(begin(pts), end(pts));
    int i= 0;
    do{
      EQUAL(bounding_rect(pts[0], pts[1], pts[2]), key);
      i++;
    } while (std::next_permutation(begin(pts), end(pts)));
    EQUAL(i,6);
  }
}
