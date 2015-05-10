// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/print-objects.hh"
#include "bitmap/alpha-map.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/iter-bmp.hh"
#include "bitmap/color-counting.hh"
#include "bitmap/quantize.hh"

void test_quantize(){
  using namespace faint;
  {
    // Single color
    Bitmap bmp(IntSize(2,2), Color(255,0,255));
    auto q(quantized(bmp, Dithering::ON));
    const auto& indices(q.map);
    const auto& colorList(q.palette);
    EQUAL(indices.GetSize(), IntSize(2,2));
    EQUAL(colorList.size(), 1);
    EQUAL(colorList[0], Color(255,0,255));

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

  {
    // Alpha-handling
    Bitmap bmp(IntSize(2, 3));
    put_pixel_raw(bmp, 0, 0, with_alpha(color_red, 0));
    put_pixel_raw(bmp, 1, 0, with_alpha(color_red, 10));
    put_pixel_raw(bmp, 0, 1, with_alpha(color_red, 20));
    put_pixel_raw(bmp, 1, 1, with_alpha(color_red, 255));
    put_pixel_raw(bmp, 0, 2, with_alpha(color_red, 20));
    put_pixel_raw(bmp, 1, 2, with_alpha(color_red, 10));
    EQUAL(count_colors(bmp), 4);
    auto mapped = quantized(bmp, Dithering::ON);
    const auto& map = mapped.map;
    const auto& palette = mapped.palette;
    ABORT_IF(palette.size() != 4);
    ABORT_IF(map.GetSize() != bmp.GetSize());

    for (int y = 0; y != map.GetSize().h; y++){
      for (int x = 0; x != map.GetSize().w; x++){
        const auto v = map.Get(x, y);
        ABORT_IF(v >= palette.size());
        auto c = palette[v];
        if (fully_transparent(c)){
          EQUAL(c, color_transparent_white);
        }
        else{
          EQUAL(get_color_raw(bmp, x, y), palette[v]);
        }
      }
    }
  }
}
