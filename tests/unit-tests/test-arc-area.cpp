// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "geo/arc.hh"
#include "geo/radii.hh"

void test_arc_area(){
  using namespace faint;
  auto e = 0.0001_eps; // Fixme

  auto ellipse_area = [](const Radii& r){
    return math::pi * r.x * r.y;
  };

  EQUAL(quadrant(0_deg), 0);
  EQUAL(quadrant(45_deg), 0);
  EQUAL(quadrant(90_deg), 1);
  EQUAL(quadrant(91_deg), 1);
  NEAR(within_quadrant(91_deg).Deg(), 1.0, e);

  NEAR(circle_arc_area(1, 360_deg), math::pi, e);
  NEAR(circle_arc_area(1, 180_deg), math::pi / 2, e);
  NEAR(circle_arc_area(1, 90_deg), math::pi / 4, e);
  NEAR(circle_arc_area(1, 45_deg), math::pi / 8, e);
  NEAR(circle_arc_area(2, 180_deg), math::pi * 2, e);
  NEAR(circle_arc_area(2, 360_deg), math::pi * 2 * 2, e);
  NEAR(arc_area(Radii(1, 1), AngleSpan(0_deg, 90_deg)), math::pi / 4, e);

  {
    auto a = arc_area(Radii(5, 2), AngleSpan(0_deg, 90_deg));
    NEAR(a, 7.85398, 0.0001_eps);
    NEAR(a, ellipse_area(Radii(5, 2)) / 4, 0.0001_eps);
  }

  NEAR(arc_area(Radii(5, 2), AngleSpan(0_deg, 45_deg)),
    5.95144,
    0.0001_eps);

  NEAR(arc_area(Radii(5, 2), AngleSpan(0_deg, 180_deg)),
    ellipse_area(Radii(5,2)) / 2.0, e);

  NEAR(arc_area(Radii(5, 2), AngleSpan(0_deg, 360_deg)),
    ellipse_area(Radii(5,2)), e);

  // Opposite radiuses
  {
    auto a = arc_area(Radii(2, 5), AngleSpan(0_deg, 90_deg));
    NEAR(a, 7.85398, 0.0001_eps);
    NEAR(a, ellipse_area(Radii(2, 5)) / 4, 0.0001_eps);
  }

  NEAR(arc_area(Radii(2, 5), AngleSpan(0_deg, 45_deg)),
    1.90253,
    0.0001_eps);

  NEAR(arc_area(Radii(2, 5), AngleSpan(0_deg, 180_deg)),
    ellipse_area(Radii(2,5)) / 2.0, e);

  NEAR(arc_area(Radii(5, 2), AngleSpan(0_deg, 360_deg)),
    ellipse_area(Radii(5,2)), e);

  // Start angles
  NEAR(arc_area(Radii(5, 2), AngleSpan(10_deg, 32_deg)),
    2.93098, 0.0001_eps);

  // Start angles
  NEAR(arc_area(Radii(2, 5), AngleSpan(10_deg, 32_deg)),
    0.87257, 0.0001_eps);
}
