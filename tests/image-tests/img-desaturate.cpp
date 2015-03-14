// -*- coding: us-ascii-unix -*-
#include "test-sys/test-name.hh"
#include "tests/test-util/file-handling.hh"
#include "tests/test-util/image-table.hh"

#include "bitmap/bitmap.hh"
#include "bitmap/filter.hh"

void img_desaturate(){
  using namespace faint;

  const Bitmap src = load_test_image(FileName("square.png"));
  Bitmap simple(src);
  desaturate_simple(simple);
  Bitmap weighted(src);
  desaturate_weighted(weighted);

  ImageTable t(get_test_name(),
    {"source", "desaturate_simple", "desaturate_weighted"});
  t.AddRow();
  t.AddCell(save_test_image(src, FileName("source.png")));
  t.AddCell(save_test_image(simple, FileName("simple.png")));
  t.AddCell(save_test_image(weighted, FileName("weighted.png")));
  save_image_table(t);
}
