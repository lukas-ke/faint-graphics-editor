// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/draw.hh"
#include "geo/offsat.hh"

void test_intersect(){
  using namespace faint;
  Bitmap dst(IntSize(10,5));
  Bitmap src(IntSize(2,3));

  // At 0,0
  VERIFY(intersects(offsat(src,0,0), onto(dst)));

  // Offset left, but still touching top-left corner of dst
  VERIFY(intersects(offsat(src,-1,-2), onto(dst)));

  // Outside to the left
  VERIFY(!intersects(offsat(src,-2,-2), onto(dst)));

  // Offset right but still touching top-right corner of dst
  VERIFY(intersects(offsat(src,9,-2), onto(dst)));

  // Outside to the right
  VERIFY(!intersects(offsat(src,10,-2), onto(dst)));

  // Offset down, left, touching lower left corner
  VERIFY(intersects(offsat(src,-1,4), onto(dst)));
  // Outside down
  VERIFY(!intersects(offsat(src,-1,5), onto(dst)));

  // Offset down, right, touching lower right corner
  VERIFY(intersects(offsat(src,9,4), onto(dst)));

  // Outside down, right
  VERIFY(!intersects(offsat(src,10,5), onto(dst)));

  // Totally outside
  VERIFY(!intersects(offsat(src,100,100), onto(dst)));
}
