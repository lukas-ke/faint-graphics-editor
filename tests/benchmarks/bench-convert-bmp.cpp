// -*- coding: us-ascii-unix -*-
#include "test-sys/bench.hh"
#include "wx/bitmap.h"
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "tests/test-util/file-handling.hh"
#include "util-wx/convert-wx.hh"

const int REPS = 1000;

void bench_convert_bmp(){
  using namespace faint;
  wxBitmap bmpWxConv(20, 20);
  wxImage imageWxConv(20, 20);
  faint::Bitmap bmpConv(faint::IntSize(20, 20));

  bmpConv = load_test_image(FileName("gauss-source.png"));
  bmpWxConv = to_wx_bmp(bmpConv);

  timed("to_faint", REPS,
    [&](){
      bmpConv = to_faint(bmpWxConv);
    });

  timed("to_wx_bmp", REPS,
    [&](){
      bmpWxConv = to_wx_bmp(bmpConv);
    });

  timed("to_wx_image", REPS,
    [&](){
      imageWxConv = to_wx_image(bmpConv);
    });
}
