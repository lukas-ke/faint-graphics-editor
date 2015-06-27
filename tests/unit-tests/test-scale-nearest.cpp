// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/bitmap-test-util.hh"
#include "tests/test-util/file-handling.hh"

#include "bitmap/scale-nearest.hh"

void test_scale_nearest(){
  using namespace faint;
  Bitmap src = load_test_image(FileName("scale-nearest-source.png"));
  Bitmap key = load_test_image(FileName("scale-nearest-key.png"));
  Bitmap dst = scale_nearest(src, {2.0, 3.0});
  VERIFY(equal(dst, key));
}
