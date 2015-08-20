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

void img_render_text(){
  using namespace faint;
  auto s = default_text_settings();
  TextInfoDC textInfo(s);
  const bool beingEdited = true;

  auto tri = tri_from_rect(Rect(Point(0,0), Size(200, 100)));
  max_width_t maxWidth(tri.Width());

  {
    Bitmap bmp({200, 200}, color_white);
    {
      FaintDC dc(bmp);
      text_lines_t lines = split_string(textInfo, "Hello world\n", maxWidth);

      render_text(dc,
        lines,
        CaretRange(0,0),
        tri_from_rect(Rect(Point(0,0), Size(200, 100))),
        beingEdited,
        textInfo,
        s);
    }
    save_test_image(bmp, FileName("render-text.png"));
  }

  {
    Bitmap bmp({200, 200}, color_white);
    {
      FaintDC dc(bmp);
      text_lines_t lines = split_string(textInfo, "Hello selected world\n",
        maxWidth);
      render_text(dc,
        lines,
        CaretRange(6, 14),
        tri,
        beingEdited,
        textInfo,
        s);
    }
    save_test_image(bmp, FileName("render-text-selection-basic.png"));
  }

  {
    auto render_selected =
      [&](const utf8_string& text, const CaretRange& r){
      Bitmap bmp({200, 200}, color_white);
      {
        text_lines_t lines = split_string(textInfo, text, maxWidth);
        FaintDC dc(bmp);
        render_text(dc,
          lines,
          r,
          tri,
          beingEdited,
          textInfo,
          s);
      }
      FileName f(no_sep("render-text-selection-",
          str_uint(r.from), "-to-", str_uint(r.to), ".png"));
      save_test_image(bmp, f);
    };

    utf8_string text("abc\ndef");
    for (size_t i = 0; i != text.size(); i++){
      render_selected(text, CaretRange(0, i));
    }
  }
}
