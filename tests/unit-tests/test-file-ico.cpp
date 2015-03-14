// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/bitmap-test-util.hh"
#include "tests/test-util/file-handling.hh"
#include "tests/test-util/print-objects.hh"
#include "bitmap/color.hh"
#include "formats/bmp/file-ico.hh"
#include "formats/bmp/bmp-types.hh"
#include "geo/int-point.hh"
#include "geo/int-rect.hh"

using namespace faint;

void check_bitmap(const Bitmap& bmp, IntSize expectedSize){
  auto size = bmp.GetSize();
  EQUAL(size, expectedSize);

  // Transparent border

  const Color green(34, 177, 76);
  const Color blue(47,54,153);

  // The icon has a 1-pixel wide transparent border
  is_uniformly(color_transparent_white,
    bmp, IntRect(IntPoint(0,0), IntSize(size.w, 1)));
  is_uniformly(color_transparent_white,
    bmp, IntRect(IntPoint(0,size.h - 1), IntSize(size.w, 1)));
  is_uniformly(color_transparent_white,
    bmp, IntRect(IntPoint(0,0), IntSize(1, size.w)));
  is_uniformly(color_transparent_white,
    bmp, IntRect(IntPoint(size.w -1,0), IntSize(1, size.h)));

  // Green top left square
  is_uniformly(green,
    bmp, IntRect(IntPoint(1, 1), size / 2));

  // Blue top right square
  is_uniformly(blue,
    bmp, IntRect(IntPoint(size.w /2, 1), size / 2));

  // Green bottom left square
  is_uniformly(blue,
    bmp, IntRect(IntPoint(1, size.h / 2), size / 2));

  // Transparent bottom right square
  is_uniformly(color_transparent_white,
    bmp, IntRect(IntPoint(size.w / 2, size.h / 2), size / 2));
}

static void check_bitmaps(const bmp_vec& bitmaps){
  ABORT_IF(bitmaps.size() != 4);
  FWD(check_bitmap(bitmaps[0], {256, 256}));
  FWD(check_bitmap(bitmaps[1], {128, 128}));
  FWD(check_bitmap(bitmaps[2], {64, 64}));
  FWD(check_bitmap(bitmaps[3], {32, 32}));
}

static std::vector<Bitmap> test_read_ico(const FilePath& filePath){
  return read_ico(filePath).Visit(
    [&](const std::vector<Bitmap>& bitmaps){
      return bitmaps;
    },
    [](const utf8_string& error) -> std::vector<Bitmap>{
      FAIL(error.c_str());
    });
}

static FilePath test_write_ico(const FileName& fileName,
  const bmp_vec& bitmaps,
  IcoCompression compression)
{
  auto path = get_test_save_path(fileName);
  ico_vec icons;
  for (const auto& bmp : bitmaps){
    icons.emplace_back(std::make_pair(bmp, compression));
  }
  write_ico(path, icons);
  return path;
}

void test_file_ico(){

  // Load the test icon
  FileName fileName("32-bpp_8-bit-alpha_no-palette-BI_RGB.ico");
  auto srcPath = get_test_load_path(fileName);
  auto originalBitmaps = test_read_ico(srcPath);

  // Verify that it looks as expected
  FWD(check_bitmaps(originalBitmaps));

  {
    // Save the icon with bmp-encoding, load the bmp and verify
    // that it is identical.
    auto filePath = test_write_ico(FileName("out-bmp.ico"),
      originalBitmaps, IcoCompression::BMP);
    FWD(check_bitmaps(test_read_ico(filePath)));
  }

  {
    // Save it with png-encoding, load the png and verify
    // that it is identical.
    auto filePath = test_write_ico(FileName("out-png.ico"),
      originalBitmaps, IcoCompression::PNG);
    FWD(check_bitmaps(test_read_ico(filePath)));
  }

}
