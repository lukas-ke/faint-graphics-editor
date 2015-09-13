// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/file-handling.hh"
#include "bitmap/scale-bicubic.hh"
#include "geo/geo-func.hh"
#include "geo/scale.hh"

void test_scale_bicubic(){
  using namespace faint;
  const Bitmap src = load_test_image(FileName("bicubic-source.png"));
  const Bitmap key = load_test_image(FileName("bicubic-key.png"));
  Bitmap dst = scale_bicubic(src, rounded(src.GetSize() * Scale(2.0, 3.0)));
  VERIFY(dst == key);
}
