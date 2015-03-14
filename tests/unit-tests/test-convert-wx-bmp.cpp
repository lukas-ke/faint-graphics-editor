// -*- coding: us-ascii-unix -*-
#include "wx/bitmap.h"
#include "wx/dcmemory.h"
#include "test-sys/test.hh"
#include "tests/test-util/print-objects.hh"
#include "tests/test-util/wx-test-util.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "util-wx/convert-wx.hh"

static faint::Color premultiplied(const faint::Color& c){
  return faint::color_from_ints(c.r * (255 - (255 - c.a)) / 255,
    c.g * (255 - (255 - c.a)) / 255,
    c.b * (255 - (255 - c.a)) / 255,
    c.a);
}

void test_convert_wx_bmp(){
  using namespace faint;

  const Color srcColor(30, 40, 50, 60);
  {
    // Bitmap <-> wxBitmap
    const Bitmap src(IntSize(8,14), srcColor);
    EQUAL(get_color_raw(src, 0, 0), srcColor);

    // Convert from faint::Bitmap to wxBitmap
    auto wxBmp(to_wx_bmp(src));
    static_assert(std::is_same<wxBitmap, decltype(wxBmp)>::value,
      "Expected to_wx_bmp to yield a wxBitmap.");
    EQUAL(to_faint(wxBmp.GetSize()), IntSize(8, 14));
    EQUAL(wxBmp.GetDepth(), 32);

    Color colorWx(get_test_platform() == TestPlatform::WINDOWS ?
      premultiplied(srcColor) : srcColor);
    EQUAL(get_color_wxBitmap(wxBmp, 0, 0), colorWx);

    // Convert back to faint::Bitmap
    auto faintBmp(to_faint(wxBmp));
    static_assert(std::is_same<faint::Bitmap, decltype(faintBmp)>::value,
      "Expected to_faint to yield a faint::Bitmap.");
    EQUAL(faintBmp.GetSize(), IntSize(8, 14));
    if (get_test_platform() == TestPlatform::WINDOWS){
      KNOWN_INEQUAL(get_color_raw(faintBmp, 0, 0), Color(30, 40, 50, 60)); // Fixme: Why alpha 255 on MSW?
    }
    else {
      EQUAL(get_color_raw(faintBmp, 0, 0), Color(30, 40, 50, 60));
    }
  }

  { // Bitmap <-> wxImage
    const Bitmap src(IntSize(8,14), Color(30, 40, 50, 60));
    auto image(to_wx_image(src));
    static_assert(std::is_same<wxImage, decltype(image)>::value,
      "Expected to_wx_image to yield a wxImage.");
    EQUAL(to_faint(image.GetSize()), IntSize(8, 14));
    VERIFY(image.HasAlpha());
    VERIFY(!image.HasMask());
    wxBitmap bmpWx(image, 32);
    KNOWN_INEQUAL(get_color_wxBitmap(bmpWx, 0, 0), premultiplied(srcColor)); // Fixme: Data loss?
  }
}
