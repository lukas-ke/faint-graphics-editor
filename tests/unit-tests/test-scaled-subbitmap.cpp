// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/bitmap-test-util.hh"
#include "tests/test-util/file-handling.hh"

#include "bitmap/draw.hh"
#include "geo/scale.hh"
#include "geo/int-rect.hh"

void test_scaled_subbitmap(){
  using namespace faint;
  const Bitmap src = load_test_image(FileName("scaled-subbitmap-source.png"));
  const Bitmap key = load_test_image(FileName("scaled-subbitmap-key.png"));
  const Bitmap badKey = load_test_image(FileName("scaled-subbitmap-bad-key.png"));

  const Bitmap dst = scaled_subbitmap(src, Scale(2.0,3.0), IntRect(IntPoint(1,1),
      IntPoint(8,3)));

  // For some reason, scaled_subbitmap is not the same as copying a rect
  // and scaling it.
  KNOWN_ERROR(equal(dst, key));
  KNOWN_ERROR(!equal(dst, badKey));


}
