// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "bitmap/draw.hh" // set_alpha
#include "bitmap/filter.hh" // desaturated_simple
#include "formats/png/file-png.hh"
#include "tests/test-util/file-handling.hh"
#include "text/utf8-string.hh"

void test_file_png(){
  using namespace faint;

  { // Load, save, reload, RGB

    read_png(get_test_load_path(FileName("square.png"))).Visit(
      [](const Bitmap& bmp){
        VERIFY(bmp.GetSize() == IntSize(185, 185));

        // Save with alpha values to verify that alpha is lost on
        // RGB-save.
        Bitmap withAlpha(bmp);
        set_alpha(withAlpha, 128);

        auto out = get_test_save_path(suffix_u8_chars(FileName("out-rgb.png")));
        auto result = write_png(out, withAlpha, PngColorType::RGB);
        ABORT_IF(result.Failed());

        read_png(out).Visit(
          [&](const Bitmap& bmp2){
            VERIFY(bmp == bmp2);
            VERIFY(withAlpha != bmp2);
          },
          [](const utf8_string& error){
            ABORT_TEST(error.c_str());
          });
      },
      [](const utf8_string& error){
        ABORT_TEST(error.c_str());
      });
  }

  { // Load, save, reload, RGBA

    read_png(get_test_load_path(FileName("square.png"))).Visit(
      [](Bitmap& bmp){
        ABORT_IF(bmp.GetSize() != IntSize(185, 185));

        // Set alpha to ensure it is retained when saving as RGBA
        set_alpha(bmp, 128);

        auto out = get_test_save_path(suffix_u8_chars(FileName("out-rgba.png")));
        auto result = write_png(out, bmp, PngColorType::RGB_ALPHA);
        ABORT_IF(result.Failed());

        read_png(out).Visit(
          [&bmp](const Bitmap& bmp2){
            VERIFY(bmp == bmp2);
          },
          [](const utf8_string& error){
            ABORT_TEST(error.c_str());
          });
      },
      [](const utf8_string& error){
        ABORT_TEST(error.c_str());
      });
  }

  { // Save, PNG_COLOR_TYPE_GRAY

    auto maybeBmp = read_png(get_test_load_path(FileName("square.png")));
    maybeBmp.Visit(
      [](const Bitmap& bmp){
        ABORT_IF(bmp.GetSize() != IntSize(185, 185));
        auto out = get_test_save_path(suffix_u8_chars(FileName("out-gray.png")));
        auto result = write_png(out, bmp, PngColorType::GRAY);
        VERIFY(result.Successful());

        auto reread = read_png(out);
        reread.Visit(
        [&bmp](const Bitmap& bmp2){
          VERIFY(desaturated_simple(bmp) == bmp2);
        },
        [](const utf8_string& error){
          ABORT_TEST(error.c_str());
        });
      },
    [](const utf8_string& error){
      ABORT_TEST(error.c_str());
    });
  }

  { // Save, PNG_COLOR_TYPE_GRAY_ALPHA

    auto maybeBmp = read_png(get_test_load_path(FileName("square.png")));
    maybeBmp.Visit(
      [](Bitmap& bmp){
        ABORT_IF(bmp.GetSize() != IntSize(185, 185));
        set_alpha(bmp, 128);

        auto out =
          get_test_save_path(suffix_u8_chars(FileName("out-gray-alpha.png")));
        auto result = write_png(out, bmp, PngColorType::GRAY_ALPHA);
        VERIFY(result.Successful());

        auto reread = read_png(out);
        reread.Visit(
        [&bmp](const Bitmap& bmp2){
          VERIFY(desaturated_simple(bmp) == bmp2);
        },
        [](const utf8_string& error){
          ABORT_TEST(error.c_str());
        });
      },
    [](const utf8_string& error){
      ABORT_TEST(error.c_str());
    });
  }

  { // Meta-data (tEXt chunks)

    Bitmap bmp(IntSize(10, 10));
    const auto textChunks = std::map<utf8_string, utf8_string>{
      {"First key", "First value"},
      {"Second key", "Second value"}};

    // Save the png with tEXt
    auto out = get_test_save_path(suffix_u8_chars(FileName("out-meta.png")));
    const auto result = write_png(out, bmp, PngColorType::RGB_ALPHA, textChunks);
    write_png(out, bmp, PngColorType::RGB_ALPHA, textChunks);
    ABORT_IF(!result.Successful());

    // Re-read the png and tEXt
    read_png_meta(out).Visit(
      [&](const Bitmap_and_tEXt& bitmapAndText){
        // Very unchanged
        VERIFY(bitmapAndText.bmp == bmp);
        VERIFY(bitmapAndText.text == textChunks);
      },
      [](const utf8_string& error){
        ABORT_TEST(error.c_str());
      });
  }
}
