// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/print-objects.hh"
#include "geo/int-point.hh"
#include "geo/int-rect.hh"
#include "geo/int-size.hh"

void test_int_rect(){
  using namespace faint;
  IntRect r(IntPoint(-1,-2), IntPoint(10,20));
  EQUAL(r.TopLeft(), IntPoint(-1,-2));
  EQUAL(r.BottomRight(), IntPoint(10,20));
  EQUAL(r.Left(), -1);
  EQUAL(r.Right(), 10);
  EQUAL(r.Top(), -2);
  EQUAL(r.Bottom(), 20);
  EQUAL(r.w, 12);
  EQUAL(r.h, 23);
  VERIFY(!empty(r));
  EQUAL(area(r), 12 * 23);
  VERIFY(r.Contains(IntPoint(-1,-2)));
  VERIFY(r.Contains(IntPoint(10,20)));
  VERIFY(!r.Contains(IntPoint(11,20)));
  VERIFY(!r.Contains(IntPoint(-2,-2)));

  IntRect r2(IntPoint(100,100), IntPoint(120,110));
  VERIFY(empty(intersection(r, r2)));

  auto u = union_of(r,r2);
  VERIFY(!empty(u));
  EQUAL(u.TopLeft(), IntPoint(-1,-2));
  EQUAL(u.BottomRight(), IntPoint(120,110));

  // IntSize constructor
  EQUAL(IntRect(IntPoint(0,0), IntSize(2,2)),
    IntRect(IntPoint(0,0), IntPoint(1,1)));

  {
    // Smallest, largest
    IntRect area1(IntPoint(0,0), IntSize(1,1));
    IntRect area4(IntPoint(0,0), IntSize(2,2));
    IntRect area2A(IntPoint(0,0), IntSize(2,1));
    IntRect area2B(IntPoint(0,0), IntSize(1,2));
    EQUAL(smallest(area1, area4), area1);
    EQUAL(largest(area1, area4), area4);
    EQUAL(largest(area2A, area2B), area2B);
    EQUAL(smallest(area2A, area2B), area2A);
  }

  { // Intersection
    using P = IntPoint;
    using R = IntRect;

    // Top left
    EQUAL(intersection(R(P(1,2), P(2,4)),
        R(P(0, 1), P(1,2))),
      R(P(1,2), P(1,2)));

    // Top right
    EQUAL(intersection(R(P(1,2), P(2,4)),
        R(P(2, 1), P(3,2))),
      R(P(2,2), P(2,2)));

    // Bottom left
    EQUAL(intersection(R(P(1,2), P(2,4)),
        R(P(0, 4), P(1,5))),
      R(P(1,4), P(1,4)));

    // Bottom right
    EQUAL(intersection(R(P(1,2), P(2,4)),
        R(P(2, 4), P(2,5))),
      R(P(2,4), P(2,4)));

    // Cover
    EQUAL(intersection(R(P(1,2), P(2,4)),
        R(P(0,1), P(3,5))),
      R(P(1,2), P(2,4)));

    // Outside left
    EQUAL(intersection(R(P(1,2), P(2,4)),
        R(P(-1,2), P(0,4))),
      IntRect(P(0,0), IntSize(0,0)));

    // Outside right
    EQUAL(intersection(R(P(1,2), P(2,4)),
        R(P(3,2), P(4,4))),
      IntRect(P(0,0), IntSize(0,0)));

    // Outside above
    EQUAL(intersection(R(P(1,2), P(2,4)),
        R(P(1,0), P(2,1))),
      IntRect(P(0,0), IntSize(0,0)));

    // Outside below
    EQUAL(intersection(R(P(1,2), P(2,4)),
        R(P(3,2), P(4,4))),
      IntRect(P(0,0), IntSize(0,0)));

  }
}
