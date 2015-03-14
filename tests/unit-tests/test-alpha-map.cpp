// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/print-objects.hh"
#include "bitmap/alpha-map.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/draw.hh"
#include "geo/int-point.hh"
#include "geo/int-size.hh"
#include "geo/int-rect.hh"
#include "geo/offsat.hh"

using namespace faint;

void test_alpha_map(){
  Bitmap bmp(IntSize(10,5));
  AlphaMap map(IntSize(10,5));

  VERIFY(intersects(offsat(map.FullReference(), IntPoint(0,0)), bmp));

  // Test offsets with AlphaMapRef of the same size as the Bitmap
  auto fullRef = map.FullReference();
  EQUAL(fullRef.GetSize(), IntSize(10,5));
  VERIFY(intersects(offsat(fullRef, 10,0), bmp));
  VERIFY(intersects(offsat(fullRef, -10,0), bmp));
  VERIFY(intersects(offsat(fullRef, 0,5), bmp));
  VERIFY(intersects(offsat(fullRef, 0,-5), bmp));

  VERIFY(!intersects(offsat(fullRef, -11,0), bmp));
  VERIFY(!intersects(offsat(fullRef, 11,0), bmp));
  VERIFY(!intersects(offsat(fullRef, 0,-6), bmp));
  VERIFY(!intersects(offsat(fullRef, 0,6), bmp));

  // Test offsets with AlphaMapRef smaller than the Bitmap
  auto ref = map.SubReference(IntRect(IntPoint(1,2), IntPoint(3,3)));
  EQUAL(ref.GetSize(), IntSize(3,2));
  VERIFY(intersects(offsat(ref, 0,0), bmp));
  VERIFY(intersects(offsat(ref, 10,5), bmp));
  VERIFY(!intersects(offsat(ref, 10,6), bmp));
  VERIFY(!intersects(offsat(ref, 11,5), bmp));

  VERIFY(intersects(offsat(ref, -3,0), bmp));
  VERIFY(!intersects(offsat(ref, -4,0), bmp));

  VERIFY(intersects(offsat(ref, 0,-2), bmp));
  VERIFY(!intersects(offsat(ref, 0,-3), bmp));

  VERIFY(intersects(offsat(ref, 10,0), bmp));
  VERIFY(!intersects(offsat(ref, 11,0), bmp));

  VERIFY(intersects(offsat(ref, 0,5), bmp));
  VERIFY(!intersects(offsat(ref, 0,6), bmp));

  // Incrementing values
  map.Add(0,0, 10);
  EQUAL(map.Get(0,0), 10);
  map.Add(0,0, 10);
  EQUAL(map.Get(0,0), 20);
  map.Add(0,0, 235);
  EQUAL(map.Get(0,0), 255);
  map.Add(0,0,1);
  EQUAL(map.Get(0,0), 255);

  // Setting values
  map.Set(0,0,100);
  EQUAL(map.Get(0,0), 100);

  map.Set(9,4,20);
  EQUAL(map.Get(9,4), 20);

  // Verify accessing values via reference
  map.Set(1,2, 10);
  EQUAL(ref.Get(0,0), 10);

  map.Set(3,2, 11);
  EQUAL(ref.Get(2,0), 11);

  map.Set(3,3, 12);
  EQUAL(ref.Get(2,1), 12);

  // Valid content in copy
  auto copy = map.SubCopy(IntRect(IntPoint(1,2), IntPoint(3,3)));
  auto copyRef = copy.FullReference();
  EQUAL(copyRef.Get(0,1), 0);
  EQUAL(copyRef.Get(2,1), 12);

  // Copy does not affect source
  copy.Set(0,0,103);
  EQUAL(copy.Get(0,0), 103);
  NOT_EQUAL(map.Get(0,0), 103);
}
