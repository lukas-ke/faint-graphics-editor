// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/print-objects.hh"
#include "bitmap/auto-crop.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/draw.hh"
#include "geo/int-point.hh"
#include "geo/int-rect.hh"
#include "geo/int-size.hh"
#include "util/at-most.hh"

using namespace faint;

struct EqualRects1{
  EqualRects1(const IntRect& r)
    : m_r(r)
  {}
  void operator()(const IntRect& r) const{
    EQUAL(m_r, r);
  }
  IntRect m_r;
};

struct EqualRects2{
  EqualRects2(const IntRect& r0, const IntRect& r1)
    : m_r0(r0),
      m_r1(r1)
  {}
  void operator()(const IntRect& r0, const IntRect& r1) const{
    EQUAL(m_r0, r0);
    EQUAL(m_r1, r1);
  }
  IntRect m_r0;
  IntRect m_r1;
};

void test_auto_crop(){
  // Blank bitmap yields no rectangles
  get_auto_crop_rectangles(Bitmap(IntSize(10,10), color_white)).Visit(
    FAIL_UNLESS_CALLED(),
    FAIL_IF_CALLED(),
    FAIL_IF_CALLED());

  // Bitmap with two differently colored halves, left and right
  Bitmap leftRight(IntSize(10,10), color_white);
  IntRect leftRect(IntPoint(0,0), IntSize(5,10));
  IntRect rightRect(IntPoint(5,0), IntSize(5,10));
  fill_rect(leftRight, leftRect, Paint(color_black));

  // "Cropped away" in left-to-right order yields
  // result rectangles in right-to-left order.
  get_auto_crop_rectangles(leftRight).Visit(
    FAIL_IF_CALLED(),
    FAIL_IF_CALLED(),
    FAIL_UNLESS_CALLED_FWD(EqualRects2(rightRect, leftRect)));

  // Bitmap with two differently colored halves, top and bottom
  Bitmap topBottom(IntSize(10,10), color_white);
  const IntRect topRect(IntPoint(0,0), IntSize(10,5));
  const IntRect bottomRect(IntPoint(0,5), IntSize(10,5));
  fill_rect(topBottom, topRect, Paint(color_black));

  // "Cropped away" in top-to-bottom order yields
  // result rectangles in bottom-to-top order.
  get_auto_crop_rectangles(topBottom).Visit(
    FAIL_IF_CALLED(),
    FAIL_IF_CALLED(),
    FAIL_UNLESS_CALLED_FWD(EqualRects2(bottomRect, topRect)));

  // Left and right halves, but with a pixel at the left edge,
  // removing it from the trimming alternatives.
  Bitmap keepLeft(leftRight);
  put_pixel(keepLeft, IntPoint(0,5), color_magenta);
  get_auto_crop_rectangles(keepLeft).Visit(
    FAIL_IF_CALLED(),
    FAIL_UNLESS_CALLED_FWD(EqualRects1(leftRect)),
    FAIL_IF_CALLED());

  // Left and right halves, but with a pixel at the right edge,
  // removing the right half from the trimming alternatives.
  Bitmap keepRight(leftRight);
  put_pixel(keepRight, IntPoint(9,5), color_magenta);
  get_auto_crop_rectangles(keepRight).Visit(
    FAIL_IF_CALLED(),
    FAIL_UNLESS_CALLED_FWD(EqualRects1(rightRect)),
    FAIL_IF_CALLED());

  // Top and bottom halves, but with a pixel at the top edge,
  // removing it from the trimming alternatives.
  Bitmap keepTop(topBottom);
  put_pixel(keepTop, IntPoint(5,0), color_magenta);
  get_auto_crop_rectangles(keepTop).Visit(
    FAIL_IF_CALLED(),
    FAIL_UNLESS_CALLED_FWD(EqualRects1(topRect)),
    FAIL_IF_CALLED());

  // Top and bottom halves, but with a pixel at the bottom edge,
  // removing it from the trimming alternatives.
  Bitmap keepBottom(topBottom);
  put_pixel(keepBottom, IntPoint(5,9), color_magenta);
  get_auto_crop_rectangles(keepBottom).Visit(
    FAIL_IF_CALLED(),
    FAIL_UNLESS_CALLED_FWD(EqualRects1(bottomRect)),
    FAIL_IF_CALLED());

  // Left and right with a pixel within each half, so that the halves
  // can't be fully cropped away (leaving two larger
  // keep-alternatives).
  Bitmap leftRightPixel(leftRight);
  put_pixel(leftRightPixel, IntPoint(2,5), color_magenta);
  put_pixel(leftRightPixel, IntPoint(7,5), color_magenta);
  get_auto_crop_rectangles(leftRightPixel).Visit(
    FAIL_IF_CALLED(),
    FAIL_IF_CALLED(),
    FAIL_UNLESS_CALLED_FWD(EqualRects2(
      IntRect(IntPoint(2,0),IntSize(8,10)),
      IntRect(IntPoint(0,0),IntSize(8,10)))));

  // Top and bottom with a pixel within each half, so that the halves
  // can't be fully cropped away (leaving two larger
  // keep-alternatives).
  Bitmap topBottomPixel(topBottom);
  put_pixel(topBottomPixel, IntPoint(5,2), color_magenta);
  put_pixel(topBottomPixel, IntPoint(5,7), color_magenta);
  get_auto_crop_rectangles(topBottomPixel).Visit(
    FAIL_IF_CALLED(),
    FAIL_IF_CALLED(),
    FAIL_UNLESS_CALLED_FWD(EqualRects2(
      IntRect(IntPoint(0,2),IntSize(10,8)),
      IntRect(IntPoint(0,0),IntSize(10,8)))));
}
