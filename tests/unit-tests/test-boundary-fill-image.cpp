// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/file-handling.hh"
#include "tests/test-util/bitmap-test-util.hh"
#include "tests/test-data/boundary-fill-source.hh"
#include "bitmap/bitmap-templates.hh"
#include "bitmap/color.hh"
#include "bitmap/draw.hh"
#include "bitmap/paint.hh"

void test_boundary_fill_image(){
  using namespace faint;

  using test::boundary_fill_source::borderColor;
  using test::boundary_fill_source::fillOrigin;
  using test::boundary_fill_source::fileName;
  using test::boundary_fill_source::fileNameKey;
  using test::boundary_fill_source::keyFillColor;

  const Bitmap src = load_test_image(FileName(fileName));
  const Bitmap key = load_test_image(FileName(fileNameKey));

  const Bitmap dst = onto_new(boundary_fill,
                                src,
                                fillOrigin,
                                Paint(keyFillColor),
                                borderColor);
  VERIFY(equal(dst, key));
}
