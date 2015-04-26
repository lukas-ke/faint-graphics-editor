// -*- coding: us-ascii-unix -*-
#include "test-sys/test-name.hh"
#include "tests/test-util/file-handling.hh"
#include "tests/test-util/image-table.hh"

#include "bitmap/scale-bicubic.hh"
#include "bitmap/scale-bilinear.hh"
#include "text/formatting.hh"

void img_scale_bicubic(){
  using namespace faint;
  Bitmap bmp = load_test_image(FileName("gauss-source.png"));

  ImageTable t(get_test_name(),
    {"Scale",
     "Bicubic",
     "Bilinear"});

  for (auto scale : {0.2, 0.5, 1.0, 1.5, 2.0}){

    const auto fileBicubic =
      save_test_image(scale_bicubic(bmp, Scale(scale, scale)),
        FileName(no_sep("bicubic-", str(scale, 1_dec), ".png"))).StripPath();

    const auto fileBilinear =
      save_test_image(scale_bilinear(bmp, Scale(scale, scale)),
        FileName(no_sep("bilinear-", str(scale, 1_dec), ".png"))).StripPath();

    t.AddToggleRow(str(scale, 1_dec), fileBicubic, fileBilinear);
  }
  save_image_table(t);
}
