// -*- coding: us-ascii-unix -*-
#include "test-sys/bench.hh"
#include "geo/primitive.hh"
#include "util/default-settings.hh"
#include "util/settings.hh"
#include "util/setting-id.hh"

faint::coord lineWidth = 0;
bool alphaBlending = false;
const int REPS = 10000000;

void bench_settings(){
  using namespace faint;

  {
    Settings s = default_line_settings();

    timed("get-linewidth-1", REPS,
      [&](){
        lineWidth = s.Get(ts_LineWidth);
        s.Set(ts_LineWidth, lineWidth + 1.0);

        alphaBlending = s.Get(ts_SwapColors);
        s.Set(ts_SwapColors, !alphaBlending);
      });
  }

  {
    Settings s = default_line_settings();

    timed("get-linewidth-2", REPS,
      [&](){
        lineWidth = s.Get(ts_LineWidth);
        s.Set(ts_LineWidth, lineWidth + 1.0);

        alphaBlending = s.Get(ts_SwapColors);
        s.Set(ts_AlphaBlending, !alphaBlending);
      });
  }
}
