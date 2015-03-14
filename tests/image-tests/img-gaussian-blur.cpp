// -*- coding: us-ascii-unix -*-
#include "test-sys/test-name.hh"
#include "tests/test-util/file-handling.hh"
#include "tests/test-util/image-table.hh"

#include "bitmap/bitmap.hh"
#include "bitmap/gaussian-blur.hh"
#include "text/formatting.hh"

void img_gaussian_blur(){
  using namespace faint;
  const Bitmap bmp = load_test_image(FileName("gauss-source.png"));

  ImageTable t(get_test_name(),
    {"Sigma",
     "gaussian_blur_exact",
     "gaussian_blur_fast"});

  for (auto sigma : {1, 2, 3, 4, 5, 6, 7, 8, 9, 10}){
    t.AddRow(str_int(sigma),
      save_test_image(gaussian_blur_exact(bmp, sigma),
        FileName(no_sep("gaussian_blur_exact_", str_int_lpad(sigma, 3), ".png"))),
      save_test_image(gaussian_blur_fast(bmp, sigma),
        FileName(no_sep("gaussian_blur_fast_", str_int_lpad(sigma, 3), ".png"))));
  }
  save_image_table(t);
}
