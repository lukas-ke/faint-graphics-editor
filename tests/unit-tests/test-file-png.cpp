// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "formats/png/file-png.hh"
#include "tests/test-util/file-handling.hh"
#include "text/utf8-string.hh"

void test_file_png(){
  using namespace faint;

  auto maybeBmp = read_png(get_test_load_path(FileName("square.png")));

  maybeBmp.Visit(
    [](const Bitmap& bmp){
      VERIFY(bmp.GetSize() == IntSize(185, 185));
      auto out = get_test_save_path(FileName("out.png"));
      auto result = write_png(out, bmp);
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
