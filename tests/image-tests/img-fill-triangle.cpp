// -*- coding: us-ascii-unix -*-
#include "tests/test-util/file-handling.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/draw.hh"
#include "bitmap/filter.hh"
#include "geo/angle.hh"
#include "geo/geo-func.hh"
#include "geo/point.hh"
#include "text/formatting.hh"

void img_fill_triangle(){
  using namespace faint;

  const Point p0(4, 22);
  const Point p1(13, 4);
  const Point p2(22, 22);
  const Point c(13, 13);
  const int NUM_IMAGES = 100;
  auto step = 360.0_deg / NUM_IMAGES;

  for (int i = 0; i != NUM_IMAGES; i++){
    Point p0r = rotate_point(p0, step * i, c);
    Point p1r = rotate_point(p1, step * i, c);
    Point p2r = rotate_point(p2, step * i, c);

    Bitmap bmp({27,27}, color_white);

    fill_triangle_color(bmp,
      rounded(p0r),
      rounded(p1r),
      rounded(p2r),
      color_black);

    save_test_image(bmp,
      FileName("triangle_" + str_int_lpad(i, 3) + ".png"));
  }
}
