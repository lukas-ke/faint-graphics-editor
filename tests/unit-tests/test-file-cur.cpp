// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/file-handling.hh"
#include "tests/test-util/text-bitmap.hh"
#include "tests/test-util/print-objects.hh"
#include "formats/bmp/file-cur.hh"

namespace faint{

std::ostream& operator<<(std::ostream& o, const std::pair<Bitmap, HotSpot>& cur){
  // Printer for EQUAL of cursor entry
  o << "Size: " << cur.first.GetSize() << " Hotspot:  " << cur.second;
  return o;
}

} // namespace

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
    // 24-bits-per-pixel, 6x6
    auto filename = FileName("24bipp-6x6.cur");
    auto cursors = test_read_cur(get_test_load_path(filename));
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

    auto savePath = get_test_save_path(suffix_u8_chars(filename));
    auto result = write_cur(savePath, cursors);
    ABORT_IF(!result.Successful());
    EQUAL(test_read_cur(savePath), cursors);
  }

  {
    // 24-bits-per-pixel, 256x256
    auto filename = FileName("24bipp-256x256.cur");
    auto cursors = test_read_cur(get_test_load_path(filename));

    ABORT_IF(cursors.size() != 1);
    const auto& cursor = cursors.front();
    EQUAL(cursor.second, HotSpot(254,254));
    EQUAL(cursor.first.GetSize(), IntSize(256,256));

    auto savePath = get_test_save_path(filename);
    auto result = write_cur(savePath, cursors);
    ABORT_IF(!result.Successful());
    EQUAL(test_read_cur(savePath), cursors);
  }

  // Fixme: Add 1-bit-per-pixel
  // Fixme: Add 4-bits-per-pixel
}
