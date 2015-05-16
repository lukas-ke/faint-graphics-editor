// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/bitmap-test-util.hh"
#include "tests/test-util/file-handling.hh"
#include "tests/test-util/print-objects.hh"
#include "tests/test-util/text-bitmap.hh"
#include "bitmap/color.hh"
#include "formats/bmp/file-bmp.hh"
#include "geo/int-point.hh"
#include "geo/int-rect.hh"

void test_file_bmp(){
  using namespace faint;
  {
    // 8-bits-per-pixel grayscale
    FileName fileName("13x7-8bipp-gray.bmp");
    auto path = get_test_load_path(fileName);
    read_bmp(path).Visit(
      [&](const Bitmap& bmp){
        EQUAL(bmp.GetSize(), IntSize(13,7));
        FWD(check(bmp,
            "XXXXXXXXXXXXX"
            "...........X."
            "..........X.."
            ".........X..."
            "........X...."
            ".......X....."
            "......X......",
            {{'.', color_white},
             {'X', color_black}}));
        auto saveResult = write_bmp(get_test_save_path(fileName), bmp,
          BitmapQuality::GRAY_8BIT);
        VERIFY(saveResult.Successful());
      },
      [&](const utf8_string& error){
        MESSAGE(path.Str().c_str());
        FAIL(error.c_str());
      });
  }

  {
    // Read 8-bits-per-pixel color
    auto path = get_test_load_path(FileName("12x6-8bipp.bmp"));
    read_bmp(path).Visit(
      [](const Bitmap& bmp){
        EQUAL(bmp.GetSize(), IntSize(12,6));
        const IntSize cellSize(6,3);
        VERIFY(is_uniformly(Color(237,28,36),
            bmp, {IntPoint(0,0), cellSize}));

        VERIFY(is_uniformly(Color(34,177,76),
            bmp, {IntPoint(6,0), cellSize}));

        VERIFY(is_uniformly(Color(77,109,243),
            bmp, {IntPoint(0,3), cellSize}));

        VERIFY(is_uniformly(Color(0,0,0),
            bmp, {IntPoint(6,3), cellSize}));
      },
      [&](const utf8_string& error){
        MESSAGE(path.Str().c_str());
        FAIL(error.c_str());
      });
  }

  {
    // Read 24-bits-per-pixel
    auto path = get_test_load_path(FileName("65x65-24bipp.bmp"));
    read_bmp(path).Visit(
      [](const Bitmap& bmp){
        EQUAL(bmp.GetSize(), IntSize(65,65));
        VERIFY(equal(bmp, load_test_image(FileName("65x65-24bipp-key.png"))));
      },
      [&](const utf8_string& error){
        MESSAGE(path.Str().c_str());
        FAIL(error.c_str());
      });
  }

  // Fixme: Add 32-bits-per-pixel
  // Fixme: Add save tests.
}
