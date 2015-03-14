// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/text-bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/draw.hh"
#include "rendering/faint-dc.hh"
#include "util/default-settings.hh"
#include "util/settings.hh"
#include "geo/int-rect.hh"

void test_faint_dc(){
  using namespace faint;

  {
    // Test "CumulativeTextWidth"
    Bitmap bmp(IntSize(100,100), color_magenta);
    FaintDC dc(bmp);

    std::vector<int> widths = dc.CumulativeTextWidth("Hello world",
      default_text_settings());

    // Ensure increasing values in each cell
    int prevWidth = widths.front();
    EQUAL(prevWidth, 0);
    for (size_t i = 1; i != widths.size(); i++){
      VERIFY(prevWidth < widths[i]);
      prevWidth = widths[i];
    }
  }

  {
    // Test "Blit"
    Bitmap bg({10,10}, color_magenta);
    FaintDC dc(bg);
    Bitmap bmp(create_bitmap({2,2},
        "rg"
        "b ",
        {{'r', color_red},
         {'g', color_green},
         {'b', color_blue},
         {' ', color_transparent_black}}));
    Settings s;
    s.Set(ts_AlphaBlending, false);
    s.Set(ts_BackgroundStyle, BackgroundStyle::SOLID);
    s.Set(ts_Bg, Paint(color_black));
    dc.Blit(bmp, {0,0}, s);

    FWD(check(subbitmap(bg, IntRect(IntPoint(0,0),IntSize(2,2))),
        "rg"
        "b ",
        {{'r', color_red},
         {'g', color_green},
         {'b', color_blue},
         {' ', color_transparent_black}}));

    clear(bg, color_magenta);
    s.Set(ts_AlphaBlending, true);
    dc.Blit(bmp, {0,0}, s);
    FWD(check(subbitmap(bg, IntRect(IntPoint(0,0),IntSize(2,2))),
        "rg"
        "b ",
        {{'r', color_red},
         {'g', color_green},
         {'b', color_blue},
         {' ', color_magenta}}));

    clear(bg, color_magenta);
    s.Set(ts_AlphaBlending, false);
    s.Set(ts_BackgroundStyle, BackgroundStyle::MASKED);
    s.Set(ts_Bg, Paint(color_green));
    dc.Blit(bmp, {0,0}, s);
    FWD(check(subbitmap(bg, IntRect(IntPoint(0,0),IntSize(2,2))),
        "rm"
        "b ",
        {{'r', color_red},
         {'m', color_magenta},
         {'b', color_blue},
         {' ', color_transparent_black}}));

    clear(bg, color_magenta);
    s.Set(ts_BackgroundStyle, BackgroundStyle::MASKED);
    s.Set(ts_AlphaBlending, true);
    dc.Blit(bmp, {0,0}, s);
    VERIFY(get_color(bg, {0,0}) == color_red);
    FWD(check(subbitmap(bg, IntRect(IntPoint(0,0),IntSize(2,2))),
        "rg"
        "b ",
        {{'r', color_red},
         {'g', color_magenta},
         {'b', color_blue},
         {' ', color_magenta}}));
  }
}
