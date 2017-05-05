// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/bitmap-test-util.hh"
#include "tests/test-util/file-handling.hh"
#include "formats/bmp/file-ico.hh"
#include "formats/png/file-png.hh" // For key image

using namespace faint;

void test_24bipp_ico(){
  auto bmps = read_ico(get_test_load_path(FileName("24bipp.ico"))).Visit(
    [](const std::vector<Bitmap>& bitmaps){
      return bitmaps;
    },
    [](const utf8_string& error) -> std::vector<Bitmap>{
      ABORT_TEST(error.c_str());
    });

  if (bmps.size() != 1){
    ABORT_TEST("Expected 1 bitmap in icon");
  }

  const auto& bmp = bmps.back();

  auto key = read_png(get_test_load_path(FileName("24bipp-key.png"))).Visit(
    [](const Bitmap& bmp){
      return bmp;
    },
    [](const utf8_string& error) -> Bitmap{
      ABORT_TEST(error.c_str());
    });

  VERIFY(key == bmp);
}
