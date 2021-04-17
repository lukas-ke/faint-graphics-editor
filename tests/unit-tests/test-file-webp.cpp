// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/file-handling.hh"

#include "formats/webp/file-webp.hh"
#include "text/utf8-string.hh"

void test_file_webp(){
  using namespace faint;

  read_webp(get_test_load_path(FileName("webp-test.webp"))).Visit(
    [](const Bitmap& bmp){
      // Certainly unexpected
      VERIFY(bmp.GetSize() == IntSize(960, 779));
    },
    [](const utf8_string& error){
      ABORT_TEST(error.c_str());
    });
}
