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

static void test_bitmap_read_write(const faint::FileName& fileName,
  faint::BitmapQuality quality,
  const faint::Bitmap& key)
{
  using namespace faint;

  read_bmp(get_test_load_path(fileName)).Visit(
    [&](const Bitmap& bmp){
      if (VERIFY(bmp == key)){

        // Save it again with specified quality
        auto savePath = get_test_save_path(suffix_u8_chars(fileName));
        auto saveResult = write_bmp(savePath, bmp, quality);
        if (VERIFY(saveResult.Successful())){
          // re-read and check
          read_bmp(savePath).Visit(
            [&key](const Bitmap& bmp){
              VERIFY(bmp == key);
            },
            [](const utf8_string& error){
              ABORT_TEST(error.c_str());
            });
        }
      }
    },
    [](const utf8_string& error){
      ABORT_TEST(error.c_str());
    });
}

void test_file_bmp(){
  using namespace faint;

  {
    // 8-bits-per-pixel grayscale
    const auto key = create_bitmap(IntSize(13,7),
      "XXXXXXXXXXXXX"
      "...........X."
      "..........X.."
      ".........X..."
      "........X...."
      ".......X....."
      "......X......",
      {{'.', color_white},
      {'X', color_black}});

    FWD(test_bitmap_read_write(FileName("13x7-8bipp-gray.bmp"),
        BitmapQuality::GRAY_8BIT,
        key));
  }

  {
    // 8-bits-per-pixel color
    const auto key = create_bitmap(IntSize(12,6),
      "RRRRRRGGGGGG"
      "RRRRRRGGGGGG"
      "RRRRRRGGGGGG"
      "BBBBBBXXXXXX"
      "BBBBBBXXXXXX"
      "BBBBBBXXXXXX",

      {{'R', Color(237,28,36)},
       {'G', Color(34,177,76)},
       {'B', Color(77,109,243)},
       {'X', color_black}});

    FWD(test_bitmap_read_write(FileName("12x6-8bipp.bmp"),
      BitmapQuality::COLOR_8BIT,
      key));
  }

  {
    // 24-bits-per-pixel
    const auto key = load_test_image(FileName("65x65-24bipp-key.png"));
    FWD(test_bitmap_read_write(FileName("65x65-24bipp.bmp"),
      BitmapQuality::COLOR_24BIT,
      key));
  }

  // Fixme: Add 32-bits-per-pixel
}
