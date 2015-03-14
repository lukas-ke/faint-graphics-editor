// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "bitmap/gradient.hh"

void test_radial_gradient(){
  using namespace faint;

  {
    // Same all
    auto rg1 = RadialGradient(Point(0,0), Radii(2,2), {{Color(255,0,255), 0.0},
          {Color(255,0,0), 1.0}});
    auto rg2 = RadialGradient(Point(0,0), Radii(2,2), {{Color(255,0,255), 0.0},
          {Color(255,0,0), 1.0}});
    VERIFY(rg1 == rg2);
    VERIFY(!(rg1 < rg2));
    VERIFY(!(rg2 > rg1));
    VERIFY(!(rg2 < rg1));
    VERIFY(!(rg1 > rg2));
    VERIFY(!(rg1 != rg2));
  }

  {
    // Different center
    auto rg1 = RadialGradient(Point(0,0), Radii(2,2), {{Color(255,0,255), 0.0},
          {Color(255,0,0), 1.0}});
    auto rg2 = RadialGradient(Point(1,0), Radii(2,2), {{Color(255,0,255), 0.0},
          {Color(255,0,0), 1.0}});
    VERIFY(rg1 < rg2);
    VERIFY(rg2 > rg1);
    VERIFY(!(rg2 < rg1));
    VERIFY(!(rg1 > rg2));
    VERIFY(rg1 != rg2);
    VERIFY(!(rg1 == rg2));
  }

  {
    // Different radii
    auto rg1 = RadialGradient(Point(0,0), Radii(0,2), {{Color(255,0,255), 0.0},
          {Color(255,0,0), 1.0}});
    auto rg2 = RadialGradient(Point(0,0), Radii(2,2), {{Color(255,0,255), 0.0},
          {Color(255,0,0), 1.0}});
    VERIFY(rg1 < rg2);
    VERIFY(rg2 > rg1);
    VERIFY(!(rg2 < rg1));
    VERIFY(!(rg1 > rg2));
    VERIFY(rg1 != rg2);
    VERIFY(!(rg1 == rg2));
  }

  {
    // Different stop count
    auto rg1 = RadialGradient(Point(0,0), Radii(0,2),
      {{Color(255,0,255), 0.0}});
    auto rg2 = RadialGradient(Point(0,0), Radii(0,2),
      {{Color(255,0,255), 0.0}, {Color(255,0,0), 1.0}});
    VERIFY(rg1 < rg2);
    VERIFY(rg2 > rg1);
    VERIFY(!(rg2 < rg1));
    VERIFY(!(rg1 > rg2));
    VERIFY(rg1 != rg2);
    VERIFY(!(rg1 == rg2));
  }

  {
    // Different stop colors
    auto rg1 = RadialGradient(Point(0,0), Radii(0,2),
      {{Color(255,0,254), 0.0}, {Color(255,0,0), 1.0}});
    auto rg2 = RadialGradient(Point(0,0), Radii(0,2),
      {{Color(255,0,255), 0.0}, {Color(255,0,0), 1.0}});
    VERIFY(rg1 < rg2);
    VERIFY(rg2 > rg1);
    VERIFY(!(rg2 < rg1));
    VERIFY(!(rg1 > rg2));
    VERIFY(rg1 != rg2);
    VERIFY(!(rg1 == rg2));
  }

  {
    // Different stop offset
    auto rg1 = RadialGradient(Point(0,0), Radii(0,2),
      {{Color(255,0,254), 0.0}, {Color(255,0,0), 0.5}});
    auto rg2 = RadialGradient(Point(0,0), Radii(0,2),
      {{Color(255,0,255), 0.0}, {Color(255,0,0), 1.0}});
    VERIFY(rg1 < rg2);
    VERIFY(rg2 > rg1);
    VERIFY(!(rg2 < rg1));
    VERIFY(!(rg1 > rg2));
    VERIFY(rg1 != rg2);
    VERIFY(!(rg1 == rg2));
  }
}
