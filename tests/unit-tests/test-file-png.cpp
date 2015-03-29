// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "formats/png/file-png.hh"
#include "tests/test-util/file-handling.hh"
#include "text/utf8-string.hh"

void test_file_png(){
  using namespace faint;

  { // Load, save, reload, RGBA

    auto maybeBmp = read_png(get_test_load_path(FileName("square.png")));
    maybeBmp.Visit(
      [](const Bitmap& bmp){
        VERIFY(bmp.GetSize() == IntSize(185, 185));
        auto out = get_test_save_path(FileName("out.png"));
        auto result = write_png(out, bmp, PngColorType::RGBA);
        VERIFY(result.Successful());

        auto reread = read_png(out);
        reread.Visit(
        [&bmp](const Bitmap& bmp2){
          VERIFY(bmp == bmp2);
        },
        [](const utf8_string& error){
          FAIL(error.c_str());
        });
      },
    [](const utf8_string& error){
      FAIL(error.c_str());
    });
  }

  { // Meta-data (tEXt chunks)

    Bitmap bmp(IntSize(10, 10));
    const auto textChunks = std::map<utf8_string, utf8_string>{
      {"First key", "First value"},
      {"Second key", "Second value"}};

    // Save the png with tEXt
    auto out = get_test_save_path(FileName("out-meta.png"));
    const auto result = write_png(out, bmp, PngColorType::RGBA, textChunks);
    write_png(out, bmp, PngColorType::RGBA, textChunks);

    if (!result.Successful()){
      FAIL(result.ErrorDescription().c_str());
    }

    // Re-read the png and tEXt
    read_png_meta(out).Visit(
      [&](const Bitmap_and_tEXt& bitmapAndText){
        // Very unchanged
        VERIFY(bitmapAndText.bmp == bmp);
        VERIFY(bitmapAndText.text == textChunks);
      },
      [](const utf8_string& error){
        FAIL(error.c_str());
      });
  }
}
