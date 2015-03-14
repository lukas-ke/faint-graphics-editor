// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/print-objects.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "geo/int-point.hh"
#include "geo/int-rect.hh"

void test_bitmap(){
  using namespace faint;
  Bitmap bmp(IntSize(100,100));
  VERIFY(bitmap_ok(bmp));
  EQUAL(bmp.GetSize(), IntSize(100,100));
  VERIFY(fully_inside(IntRect(IntPoint(0,0),IntPoint(99,99)), bmp));
  VERIFY(!fully_inside(IntRect(IntPoint(-1,0),IntPoint(99,99)), bmp));
  VERIFY(!fully_inside(IntRect(IntPoint(0,-1),IntPoint(99,99)), bmp));
  VERIFY(!fully_inside(IntRect(IntPoint(0,0),IntPoint(100,99)), bmp));
  VERIFY(!fully_inside(IntRect(IntPoint(0,0),IntPoint(99,100)), bmp));
  VERIFY(point_in_bitmap(bmp, IntPoint(0,0)));
  VERIFY(point_in_bitmap(bmp, IntPoint(99,99)));
  VERIFY(!point_in_bitmap(bmp, IntPoint(-1,0)));
  VERIFY(!point_in_bitmap(bmp, IntPoint(0,-1)));
  VERIFY(!point_in_bitmap(bmp, IntPoint(100,99)));
  VERIFY(!point_in_bitmap(bmp, IntPoint(99,100)));

  EQUAL(get_color(bmp, IntPoint(0,0)), Color(0,0,0,0));
  VERIFY(is_blank(bmp));
  put_pixel(bmp, IntPoint(0,0), Color(255,0,255,0));
  VERIFY(!is_blank(bmp));
  EQUAL(get_color(bmp, IntPoint(0,0)), Color(255,0,255,0));
  EQUAL(get_color(bmp, IntPoint(1,0)), Color(0,0,0,0));

  Bitmap bmp2;
  VERIFY(!bitmap_ok(bmp2));
  VERIFY(!point_in_bitmap(bmp2, IntPoint(0,0)));

  bmp2 = bmp;
  VERIFY(bitmap_ok(bmp));
  VERIFY(bitmap_ok(bmp2));
  EQUAL(bmp2.GetSize(), bmp.GetSize());

  Bitmap bmp3(IntSize(10,10));
  bmp3.Swap(bmp2);
  EQUAL(bmp3.GetSize(), IntSize(100,100));
}
