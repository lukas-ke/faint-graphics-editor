// -*- coding: us-ascii-unix -*-
#include "test-sys/test-name.hh"
#include "tests/test-util/file-handling.hh"
#include "tests/test-util/image-table.hh"

#include "bitmap/bitmap.hh"
#include "bitmap/filter.hh"
#include "text/formatting.hh"

void img_unsharp_mask(){
  using namespace faint;
  FileName sourceFile("gauss-source.png");
  const Bitmap bmp = load_test_image(sourceFile);
  save_test_image(bmp, sourceFile);

  ImageTable t(get_test_name(), {"Sigma", "unsharp_mask_fast", "source"});
  for (auto sigma : {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}){
    t.AddRow(str_int(sigma),
      save_test_image(unsharp_mask_fast(bmp, (double)sigma),
        FileName(no_sep("unsharp_mask_fast_", str_int_lpad(sigma, 3), ".png"))),
      sourceFile);
  }

  save_image_table(t);
}
