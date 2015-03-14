// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "wx/colour.h"
#include "wx/geometry.h"
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "geo/int-point.hh"
#include "geo/int-rect.hh"
#include "tests/test-util/print-objects.hh"
#include "util-wx/convert-wx.hh"

void test_convert_wx(){
  using namespace faint;

  { // Color <-> wxColour
    const Color srcColor(30, 40, 50, 60);
    auto colWx = to_wx(srcColor);
    static_assert(std::is_same<wxColour, decltype(colWx)>::value,
      "Expected to_wx to yield a wxColour.");
    EQUAL(colWx.Red(), srcColor.r);
    EQUAL(colWx.Green(), srcColor.g);
    EQUAL(colWx.Blue(), srcColor.b);
    EQUAL(colWx.Alpha(), srcColor.a);
    EQUAL(to_faint(colWx), srcColor);
  }

  { // ColorRGB <-> wxColour
    const ColRGB srcColor(30, 40, 50);
    auto colWx = to_wx(srcColor);
    static_assert(std::is_same<wxColour, decltype(colWx)>::value,
      "Expected to_wx to yield a wxColour.");
    EQUAL(colWx.Red(), srcColor.r);
    EQUAL(colWx.Green(), srcColor.g);
    EQUAL(colWx.Blue(), srcColor.b);
    EQUAL(colWx.Alpha(), 255);
    EQUAL(strip_alpha(to_faint(colWx)), srcColor);
  }

  { // Positive IntPoint <-> wxPoint
    const IntPoint pt(20,30);
    wxPoint ptWx(to_wx(pt));
    EQUAL(ptWx.x, pt.x);
    EQUAL(ptWx.y, pt.y);
    EQUAL(to_faint(ptWx), pt);
  }

  { // Negative IntPoint <-> wxPoint
    const IntPoint pt(-20,-30);
    wxPoint ptWx(to_wx(pt));
    EQUAL(ptWx.x, pt.x);
    EQUAL(ptWx.y, pt.y);
    EQUAL(to_faint(ptWx), pt);
  }

  { // IntSize <-> wxSize
    const IntSize sz(20,30);
    wxSize szWx(to_wx(sz));
    EQUAL(szWx.GetWidth(), sz.w);
    EQUAL(szWx.GetHeight(), sz.h);
    EQUAL(to_faint(szWx), sz);
  }

  { // IntRect <-> wxRect
    const IntRect srcRect(IntPoint(20,30), IntPoint(25, 32));
    wxRect rectWx(to_wx(srcRect));
    EQUAL(to_faint(rectWx.GetSize()), srcRect.GetSize());
    EQUAL(to_faint(rectWx.GetTopLeft()), srcRect.TopLeft());
  }

}
