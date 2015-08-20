// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/file-handling.hh"
#include "bitmap/color.hh"
#include "geo/rect.hh"
#include "geo/size.hh"
#include "geo/tri.hh"
#include "rendering/render-text.hh"
#include "rendering/text-info-dc.hh"
#include "util/default-settings.hh"
#include "text/formatting.hh"
#include "text/char-constants.hh"
#include "text/split-string.hh"

void img_render_text_mask(){
  using namespace faint;
  auto s = default_text_settings();
  TextInfoDC textInfo(s);

  // Fixme: Seems render_text_mask is rather broken.
  auto tri = tri_from_rect(Rect(Point(0,0), Size(200, 100)));
  max_width_t maxWidth(tri.Width());

  {
    Bitmap bmp({200, 200}, color_white);
    {
      FaintDC dc(bmp);
      text_lines_t lines = split_string(textInfo, "Hello world\nAnd such\netc",
        maxWidth);

      render_text_mask(dc,
        lines,
        tri_from_rect(Rect(Point(0,0), Size(200, 100))),
        textInfo,
        s,
        Paint(Color(255,0,255)), // In tri
        Paint(Color(0,0,0))); // In text

    }
    save_test_image(bmp, FileName("render-text-mask.png"));
  }
}
