// -*- coding: us-ascii-unix -*-
#include "test-sys/bench.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/draw.hh"
#include "geo/point-range.hh"

static faint::Bitmap bmp(faint::IntSize(640,480));

void range_loop(){
  using namespace faint;
  const Color r(255,0,0);
  const int x0 = 0;
  const int x1 = 639;
  const int y0 = 0;
  const int y1 = 479;
  for (const auto pt : point_range({x0, y0},{x1, y1})){
    put_pixel(bmp, pt, r);
  }
}

void bench_point_range(){
  using namespace faint;
  const Color r(255,0,0);
  const int reps = 200;
  const int x0 = 0;
  const int x1 = 639;
  const int y0 = 0;
  const int y1 = 479;

  TIMED(range_loop, reps);

  timed("Range loop (lambda)", reps, [=](){
    for (const auto pt : point_range({x0, y0},{x1, y1})){
      put_pixel(bmp, pt, r);
    }});

  timed("Normal loop", reps, [=](){
    for (int y = y0; y <= y1; y++){
      for (int x = x0; x <= x1; x++){
        put_pixel(bmp, IntPoint(x, y), r);
      }
    }});

  timed("Raw loop", reps, [=](){
    for (int y = y0; y <= y1; y++){
      for (int x = x0; x <= x1; x++){
        put_pixel_raw(bmp, x, y, r);
      }
    }});
}
