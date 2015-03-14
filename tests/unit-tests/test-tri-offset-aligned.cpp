// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "geo/tri.hh"

void verify_tri(const faint::Tri& t, const faint::Point& p0, const faint::Point& p1, const faint::Point& p2){
  const auto eps = 0.001_eps;
  NEAR(t.P0().x, p0.x, eps);
  NEAR(t.P1().x, p1.x, eps);
  NEAR(t.P0().y, p0.y, eps);
  NEAR(t.P2().y, p2.y, eps);
  NEAR(t.P2().y, p2.y, eps);
}

#define VERIFY_TRI FWY(verify_tri)

void test_tri_offset_aligned(){
  using namespace faint;

  // <../../doc/test-tri-legend.png>

  {
    // <../../doc/test-tri-a-1.png>
    const Tri t({0,0}, {10,0}, {0,8});

    FWD(verify_tri(offset_aligned(t, 3.0, 2.0),
        {3.0, 2.0}, {13.0, 2.0}, {3.0, 10.0}));

    FWD(verify_tri(offset_aligned(t, -3, -2.0),
        {-3.0, -2.0}, {7.0, -2.0}, {-3.0, 6.0}));
  }

  {
    // <../../doc/test-tri-a-2.png>
    const Tri t(Point(0,0), Point(0,10), Point(-10,0));

    FWD(verify_tri(offset_aligned(t, 3.0, 2.0),
        {-2.0, 3.0}, {-2.0, 13.0}, {-12.0, 3.0}));

    FWD(verify_tri(offset_aligned(t, -3, -2.0),
        {2.0,-3.0}, {2.0,7.0}, {-8.0,-3.0}));
  }

  {
    // <../../doc/test-tri-a-3.png>
    const Tri t(Point(0,0), Point(-10,0), Point(0,-8));

    FWD(verify_tri(offset_aligned(t, 3.0, 2.0),
        {-3,-2}, {-13,-2}, {-3,-10}));

    FWD(verify_tri(offset_aligned(t, -3, -2.0),
        {3,2}, {-7,2}, {3,-6}));
  }

  {
    // <../../doc/test-tri-a-4.png>
    const Tri t(Point(0,0), Point(0,-10), Point(8,0));

    FWD(verify_tri(offset_aligned(t, 3.0, 2.0),
        {2.0, -3.0}, {2.0, -13.0}, {10.0,-3.0}));

    FWD(verify_tri(offset_aligned(t, -3, -2.0),
        {-2.0, 3.0}, {-2.0,-7.0}, {6.0,3.0}));
  }

  {
    // <../../doc/test-tri-b-1.png>
    const Tri t(Point(0,0), Point(-10,0), Point(0,8));

    FWD(verify_tri(offset_aligned(t, 3.0, 2.0),
        {-3.0, 2.0}, {-13.0, 2.0}, {-3.0, 10.0}));

    FWD(verify_tri(offset_aligned(t, -3, -2.0),
        {3.0, -2.0}, {-7.0, -2.0}, {3.0, 6.0}));
  }

  {
    // <../../doc/test-tri-b-2.png>
    const Tri t(Point(0,0), Point(0,-10), Point(-8,0));

    FWD(verify_tri(offset_aligned(t, 3.0, 2.0),
        {-2.0,-3.0}, {-2.0, -13.0}, {-10.0,-3.0}));

    FWD(verify_tri(offset_aligned(t, -3, -2.0),
        {2.0, 3.0}, {2.0, -7.0}, {-6.0, 3.0}));
  }

  {
    // <../../doc/test-tri-b-3.png>
    const Tri t(Point(0,0), Point(10,0), Point(0,-8));

    FWD(verify_tri(offset_aligned(t, 3.0, 2.0),
        {3.0, -2.0}, {13.0, -2.0}, {3.0, -10.0}));

    FWD(verify_tri(offset_aligned(t, -3, -2.0),
        {-3.0, 2.0}, {7.0, 2.0}, {-3.0, -6.0}));
  }

  {
    // <../../doc/test-tri-b-4.png>
    const Tri t(Point(0, 0), Point(0, 10), Point(8, 0));

    FWD(verify_tri(offset_aligned(t, 3.0, 2.0),
        {2.0, 3.0}, {2.0, 13.0}, {10.0, 3.0}));

    FWD(verify_tri(offset_aligned(t, -3, -2.0),
        {-2.0, -3.0}, {-2.0, 7.0}, {6.0, -3.0}));
  }
}
