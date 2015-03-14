// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/text-bitmap.hh"
#include "tests/test-util/print-objects.hh"
#include "bitmap/channel.hh"
#include "bitmap/iter-bmp.hh"

void test_channel(){
  using namespace faint;

  const Color A(74,209,205);
  const Color B(0,0,0);
  const Color C(255,255,255);
  const Color D(193,64,184,120);

  Bitmap bmp(create_bitmap({3,4},
      "ABC"
      "DDD"
      "DDD"
      "ABC",
      {{'A', A},
       {'B', B},
       {'C', C},
       {'D', D}}));

  Channels ch = separate_into_channels(bmp);
  EQUAL(resigned(ch.r.size()),area(bmp.GetSize()));
  EQUAL(ch.w, bmp.m_w);

  VERIFY(get_colors(ch) ==
    std::vector<Color>({
        A,B,C,
        D,D,D,
        D,D,D,
        A,B,C}));

  Bitmap bmp2 = combine_into_bitmap(ch);
  EQUAL(bmp2.GetSize(), bmp.GetSize());

  for (ITER_XY(x, y, bmp)){
    EQUAL(get_color_raw(bmp, x, y), get_color_raw(bmp2, x, y));
  }
}
