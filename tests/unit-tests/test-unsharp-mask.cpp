// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/filter.hh"
#include "tests/test-util/file-handling.hh"

void test_unsharp_mask(){
  using namespace faint;
  const auto bmp =
    unsharp_mask_fast(load_test_image(FileName("alpha-128.png")), 2.0);

  // Verify that alpha is not lost
  VERIFY(translucent(get_color_raw(bmp, 0, 0)));
}
