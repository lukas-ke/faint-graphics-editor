// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/file-handling.hh"
#include "geo/geo-func.hh"
#include "geo/scale.hh"

#include "bitmap/scale-bilinear.hh"

void test_scale_bilinear(){
  using namespace faint;
  const Bitmap src = load_test_image(FileName("bicubic-source.png"));
  const Bitmap key = load_test_image(FileName("bilinear-key.png"));
  Bitmap dst = scale_bilinear(src, rounded(src.GetSize() * Scale(2.0, 3.0)));
  VERIFY(dst == key);
}
