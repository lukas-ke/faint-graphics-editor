// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/print-objects.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/paint.hh"
#include "geo/int-point.hh"
#include "util/either.hh"

using namespace faint;
using either_t = Either<Bitmap, Color>;

void test_either(){
  either_t either1(Color(255,0,255));
  Paint paint = either1.Visit(
    [](const Bitmap&) -> Paint{
      FAIL();
    },
    [](const Color& c){
      return Paint(c);
    });
  ASSERT(paint.IsColor());
  EQUAL(paint.GetColor(), Color(255,0,255));

  either_t either2(either1);
  paint = either2.Visit(
    [](const Bitmap&) -> Paint{
      FAIL();
    },
    [](const Color& c){
      return Paint(c);
    });
  ASSERT(paint.IsColor());
  EQUAL(paint.GetColor(), Color(255,0,255));
  VERIFY(either2.Get<Bitmap>().NotSet());
  VERIFY(either2.Get<Color>().IsSet());

  either_t either3(Bitmap(IntSize(10,10), Color(255,0,0)));
  paint = either3.Visit(
    [](const Bitmap& bmp){
      return Paint(get_color(bmp, IntPoint(1,1)));
    },
    [](const Color&) -> Paint{
      FAIL();
    });
  ASSERT(paint.IsColor());
  EQUAL(paint.GetColor(), Color(255,0,0));

  either3.Set(Bitmap(IntSize(10,10), Color(0,0,255)));
  paint = either3.Visit(
    [](const Bitmap& bmp){
      return Paint(get_color(bmp, IntPoint(1,1)));
    },
    [](const Color&) -> Paint{
      FAIL();
    });
  ASSERT(paint.IsColor());
  EQUAL(paint.GetColor(), Color(0,0,255));

  either3.Set(Color(0,255,0));
  paint = either3.Visit(
    [](const Bitmap&) -> Paint{
      FAIL();
    },
    [](const Color& c){
      return Paint(c);
    });
  ASSERT(paint.IsColor());
  EQUAL(paint.GetColor(), Color(0,255,0));

  VERIFY(either3.Get<Bitmap>().NotSet());

  ASSERT(either3.Get<Color>().IsSet());
  EQUAL(either3.Get<Color>().Get(), Color(0,255,0));

  {
    using T = Either<Color, IntSize>;

    // Not equal, IntSize
    VERIFY(T(IntSize(10, 10)) != T(IntSize(10, 11)));
    VERIFY(!(T(IntSize(10, 10)) == T(IntSize(10, 11))));

    // Equal, IntSize
    VERIFY(T(IntSize(10, 10)) == T(IntSize(10, 10)));
    VERIFY(!(T(IntSize(10, 10)) != T(IntSize(10, 10))));

    // Not-equal, Color
    VERIFY(T(Color(255, 0, 255)) != T(Color(255, 0, 254)));
    VERIFY(!(T(Color(255, 0, 255)) == T(Color(255, 0, 254))));

    // Equal Color
    VERIFY(T(Color(255, 0, 255)) == T(Color(255, 0, 255)));
    VERIFY(!(T(Color(255, 0, 255)) != T(Color(255, 0, 255))));
  }
}
