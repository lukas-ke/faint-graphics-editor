// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "bitmap/color.hh"
#include "util/range-iter.hh"
#include "util/make-vector.hh"

void test_range_iter(){
  using namespace faint;
  auto v = make_vector(make_closed_range(0,255), grayscale_rgb);
  EQUAL(v.size(), 256);
  VERIFY(v[0] == ColRGB(0,0,0));
  VERIFY(v[128] == ColRGB(128,128,128));
  VERIFY(v[255] == ColRGB(255,255,255));
}
