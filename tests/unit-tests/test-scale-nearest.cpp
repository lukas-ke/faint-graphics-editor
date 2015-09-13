// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/bitmap-test-util.hh"
#include "tests/test-util/file-handling.hh"

#include "bitmap/scale-nearest.hh"
#include "geo/geo-func.hh"
#include "geo/scale.hh"

void test_scale_nearest(){
  using namespace faint;
  const Bitmap src = load_test_image(FileName("scale-nearest-source.png"));
  const Bitmap key = load_test_image(FileName("scale-nearest-key.png"));
  const Bitmap dst = scale_nearest(src, rounded(src.GetSize() * Scale(2.0, 3.0)));
  VERIFY(equal(dst, key));
  VERIFY(equal(scale_nearest(src, 2),
    scale_nearest(src, rounded(src.GetSize() * Scale(2.0)))));
}
