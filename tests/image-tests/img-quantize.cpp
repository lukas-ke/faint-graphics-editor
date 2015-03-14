// -*- coding: us-ascii-unix -*-
#include "tests/test-util/file-handling.hh"

#include "bitmap/bitmap.hh"
#include "bitmap/quantize.hh"
#include "geo/int-size.hh"
#include "text/formatting.hh"

void img_quantize(){
  using namespace faint;
  {
    Bitmap bmp = load_test_image(FileName("gauss-source.png"));
    save_test_image(quantized_bmp(bmp, Dithering::ON, OctTreeDepth::FOUR),
      FileName("gauss-source-quantized-depth-4-dithering.png"));
    save_test_image(quantized_bmp(bmp, Dithering::OFF, OctTreeDepth::FOUR),
      FileName("gauss-source-quantized-depth-4-no-dithering.png"));

    save_test_image(quantized_bmp(bmp, Dithering::ON, OctTreeDepth::FIVE),
      FileName("gauss-source-quantized-depth-5-dithering.png"));
    save_test_image(quantized_bmp(bmp, Dithering::OFF, OctTreeDepth::FIVE),
      FileName("gauss-source-quantized-depth-5-no-dithering.png"));

    save_test_image(quantized_bmp(bmp, Dithering::ON, OctTreeDepth::SIX),
      FileName("gauss-source-quantized-depth-6-dithering.png"));
    save_test_image(quantized_bmp(bmp, Dithering::OFF, OctTreeDepth::SIX),
      FileName("gauss-source-quantized-depth-6-no-dithering.png"));
  }


  {
    Bitmap bmp = load_test_image(FileName("gradients.png"));
    save_test_image(quantized_bmp(bmp, Dithering::ON, OctTreeDepth::FOUR),
      FileName("gradients-quantized-depth-4-dithering.png"));
    save_test_image(quantized_bmp(bmp, Dithering::OFF, OctTreeDepth::FOUR),
      FileName("gradients-quantized-depth-4-no-dithering.png"));

    save_test_image(quantized_bmp(bmp, Dithering::ON, OctTreeDepth::FIVE),
      FileName("gradients-quantized-depth-5-dithering.png"));
    save_test_image(quantized_bmp(bmp, Dithering::OFF, OctTreeDepth::FIVE),
      FileName("gradients-quantized-depth-5-no-dithering.png"));

    save_test_image(quantized_bmp(bmp, Dithering::ON, OctTreeDepth::SIX),
      FileName("gradients-quantized-depth-6-dithering.png"));
    save_test_image(quantized_bmp(bmp, Dithering::OFF, OctTreeDepth::SIX),
      FileName("gradients-quantized-depth-6-no-dithering.png"));
  }
}
