// -*- coding: us-ascii-unix -*-
#include "test-sys/bench.hh"
#include "tests/test-util/file-handling.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/gaussian-blur.hh"
#include "text/formatting.hh"

static faint::Bitmap bmp;

const int REPS = 5;

static void timed_gaussian_blur_exact(int sigma){
  using namespace faint;
  auto title = no_sep("gaussian_blur_exact(", str_int(sigma), ")");
  timed(title.c_str(), REPS, [&](){gaussian_blur_exact(bmp, sigma);});
}

static void timed_gaussian_blur_fast(int sigma){
  using namespace faint;
  auto title = no_sep("gaussian_blur_fast(", str_int(sigma), ")");
  timed(title.c_str(), REPS, [&](){gaussian_blur_fast(bmp, sigma);});
}

void bench_gaussian_blur(){
  using namespace faint;
  bmp = load_test_image(FileName("gauss-source.png"));
  timed_gaussian_blur_exact(1);
  timed_gaussian_blur_exact(5);
  timed_gaussian_blur_exact(10);
  timed_gaussian_blur_fast(1);
  timed_gaussian_blur_fast(5);
  timed_gaussian_blur_fast(10);
}
