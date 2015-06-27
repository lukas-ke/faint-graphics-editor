// -*- coding: us-ascii-unix -*-
#include "bitmap/draw.hh"
#include "test-sys/bench.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "tests/test-data/boundary-fill-source.hh"
#include "tests/test-util/file-handling.hh"
#include "tests/test-util/file-handling.hh"
#include "tests/test-util/bitmap-test-util.hh"
#include "bitmap/bitmap-templates.hh"
#include <cassert>

const int REPS = 10;

void bench_boundary_fill(){
  using namespace faint;

  using test::boundary_fill_source::borderColor;
  using test::boundary_fill_source::fillOrigin;
  using test::boundary_fill_source::fileName;

  const Bitmap src = load_test_image(FileName(fileName));
  const Paint fill(test::boundary_fill_source::keyFillColor);

  Bitmap copy;

  timed("boundary_fill", REPS,
    [&](){
      copy = src;
      boundary_fill(copy, fillOrigin,
        fill, borderColor);
    });
}
