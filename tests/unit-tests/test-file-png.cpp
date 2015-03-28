// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "formats/bmp/file-bmp.hh"
#include "formats/png/file-png.hh"
#include "tests/test-util/file-handling.hh"
#include "text/utf8-string.hh"

void test_file_png(){
  using namespace faint;

  auto maybeBmp = read_png(get_test_load_path(FileName("square.png")));

  maybeBmp.Visit(
    [](const Bitmap& bmp){
      VERIFY(bmp.GetSize() == IntSize(185, 185));
      auto result = write_bmp(get_test_save_path(FileName("out.bmp")),
        bmp,
        BitmapQuality::COLOR_24BIT);
      VERIFY(result.Successful());
    },
    [](const utf8_string& error){
      FAIL(error.c_str());
    });
}
