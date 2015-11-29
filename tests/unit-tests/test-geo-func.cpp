// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/print-objects.hh"
#include "geo/geo-func.hh"
#include "geo/point.hh"
#include "geo/scale.hh"

void test_geo_func(){
  using namespace faint;

  { // scale_point
    using Pt = Point;
    using Sc = Scale;

    EQUAL(scale_point(Pt{1.0, 1.0}, Sc{2.0, 4.0}, Pt{2.0, 2.0}),
      Pt(0.0, -2.0));

    EQUAL(scale_point(Pt{2.0, 2.0}, Sc{2.0, 4.0}, Pt{2.0, 2.0}),
      Pt(2.0, 2.0));

    EQUAL(scale_point(Pt{3.0, 3.0}, Sc{2.0, 4.0}, Pt{2.0, 2.0}),
      Pt(4.0, 6.0));
  }
}
