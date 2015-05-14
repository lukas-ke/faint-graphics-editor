// -*- coding: us-ascii-unix -*-
#include "test-sys/bench.hh"
#include "tests/test-util/file-handling.hh"
#include "bitmap/color-counting.hh"

const int REPS = 5;
namespace {
  faint::color_counts_t out;
}

void bench_color_counting(){
  using namespace faint;
  auto bmp = load_test_image(FileName("gauss-source.png"));
  {
    color_counts_t colors;
    timed("count_colors", REPS, [&](){add_color_counts(bmp, colors);});
    out = colors;
  }
}
