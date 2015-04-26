// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/file-handling.hh"

#include "bitmap/scale-bicubic.hh"

void test_scale_bicubic(){
  using namespace faint;
  Bitmap src = load_test_image(FileName("bicubic-source.png"));
  Bitmap dst = scale_bicubic(src, {2.0, 2.0});
  VERIFY(dst.m_w == src.m_w * 2);
}
