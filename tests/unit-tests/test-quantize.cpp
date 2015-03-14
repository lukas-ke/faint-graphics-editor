// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/print-objects.hh"
#include "bitmap/alpha-map.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/color-counting.hh"
#include "bitmap/quantize.hh"

void test_quantize(){
  using namespace faint;
  {
    // Single color
    Bitmap bmp(IntSize(2,2), Color(255,0,255));
    auto q(quantized(bmp, Dithering::ON));
    const auto& indices(q.first);
    const auto& colorList(q.second);
    EQUAL(indices.GetSize(), IntSize(2,2));
    EQUAL(colorList.GetNumColors(), 1);
    EQUAL(colorList.GetColor(0), Color(255,0,255));

    for (int y = 0; y != 2; y++){
      for (int x = 0; x != 2; x++){
        EQUAL(indices.Get(x,y), 0);
      }
    }
  }

  {
    // Verify that <= 256 color bitmaps are unmodified
    Bitmap bmp(IntSize(256, 1));
    for (int x = 0; x != 256; x++){
      put_pixel_raw(bmp, x, 0, color_from_ints(x, 128,255 - x));
    }
    EQUAL(count_colors(bmp), 256);
    Bitmap q(quantized_bmp(bmp, Dithering::ON));
    EQUAL(count_colors(q), 256);
    VERIFY(bmp == q);
  }

  {
    // Provokes \ref(quantize-error)
    Bitmap bmp(IntSize(512, 1));
    for (int x = 0; x != 255; x++){
      put_pixel_raw(bmp, x, 0, color_from_ints(x, 0,0));
    }
    for (int x = 256; x != 512; x++){
      put_pixel_raw(bmp, x, 0, color_from_ints(255, x - 256, 0));
    }

    EQUAL(count_colors(bmp), 512);
    Bitmap q(bmp);
    quantize(q, Dithering::ON);
    VERIFY(count_colors(q) <= 256);
  }
}
