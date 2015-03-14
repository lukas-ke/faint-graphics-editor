// -*- coding: us-ascii-unix -*-
#include "test-sys/test-name.hh"
#include "tests/test-util/file-handling.hh"
#include "tests/test-util/image-table.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/filter.hh"
#include "text/formatting.hh"

void img_brightness_contrast(){
  using namespace faint;
  const Bitmap bmp = load_test_image(FileName("square.png"));

  const std::vector<double> brightness_values =
    {-0.6, -0.4, -0.2, 0.0, 0.2, 0.4, 0.6};
  const std::vector<double> contrast_values =
    {0.0, 0.2, 0.4, 0.6, 0.8, 1.0, 1.2, 1.4, 1.6, 1.8, 2.0};

  std::vector<utf8_string> headings;
  headings.emplace_back("");
  for (auto b : contrast_values){
    headings.emplace_back(str(b, 2_dec));
  }
  headings.emplace_back("(contrast)");

  ImageTable t(get_test_name(), headings);

  for (double brightness : brightness_values){
    t.AddRow();
    t.AddCell(str(brightness, 2_dec));
    for (double contrast : contrast_values){
      t.AddCell(
        save_test_image(brightness_and_contrast(bmp, {brightness, contrast}),
          FileName("b" + str(brightness, 2_dec) + "c" +
            str(contrast, 2_dec) + ".png")));
    }
  }
  save_image_table(t);
}
