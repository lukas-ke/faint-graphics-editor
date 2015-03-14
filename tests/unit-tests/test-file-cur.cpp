// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/file-handling.hh"
#include "tests/test-util/text-bitmap.hh"
#include "tests/test-util/print-objects.hh"
#include "formats/bmp/file-cur.hh"

using namespace faint;

static cur_vec test_read_cur(const FilePath& filePath){
  return read_cur(filePath).Visit(
    [&](const cur_vec& cursors){
      return cursors;
    },
    [&](const utf8_string& error) -> cur_vec{
      FAIL(filePath.Str().c_str(), error.c_str());
    });
}

void test_file_cur(){

  {
    // 24bpp, 6x6
    auto cursors = test_read_cur(get_test_load_path(FileName("24bpp-6x6.cur")));
    ABORT_IF(cursors.size() != 1);
    const auto& cursor = cursors.front();

    EQUAL(cursor.second, HotSpot(4,1));
    EQUAL(cursor.first.GetSize(), IntSize(6,6));
    FWD(check(cursor.first,
        "......"
        ".XXXX."
        "...XX."
        "..X.X."
        ".X..X."
        "......",
        // Fixme: Why black? ico is color_transparent_white. :(
        {{'.', color_transparent_black},
         {'X', color_black}}));
  }

  {
    // 24bpp, 256x256
    auto cursors = test_read_cur(get_test_load_path(
      FileName("24bpp-256x256.cur")));

    ABORT_IF(cursors.size() != 1);
    const auto& cursor = cursors.front();
    EQUAL(cursor.second, HotSpot(254,254));
    EQUAL(cursor.first.GetSize(), IntSize(256,256));
  }

  // Fixme: Add 1bpp
  // Fixme: Add 4bpp
}
