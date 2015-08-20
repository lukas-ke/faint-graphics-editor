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

namespace faint{

class StubExpressionContext : public ExpressionContext{
  Optional<Calibration> GetCalibration() const override{
    return no_option();
  }

  const Object* GetObject(const utf8_string&) const override{
    return nullptr;
  }
};

TextBuffer& selected_range(TextBuffer& text, const CaretRange& r){
  text.select(r);
  return text;
}

} // namespace

void img_render_text(){
  using namespace faint;
  auto s = default_text_settings();
  TextInfoDC textInfo(s);
  const bool beingEdited = true;

  {
    Bitmap bmp({200, 200}, color_white);
    {
      FaintDC dc(bmp);

      StubExpressionContext ctx;
      TextBuffer text("Hello world\n");
      render_text(dc,
        text,
        no_option(),
        tri_from_rect(Rect(Point(0,0), Size(200, 100))),
        beingEdited,
        textInfo,
        ctx,
        s);
    }
    save_test_image(bmp, FileName("render-text.png"));
  }

  {
    Bitmap bmp({200, 200}, color_white);
    {
      FaintDC dc(bmp);

      StubExpressionContext ctx;
      TextBuffer text("Hello selected world\n");
      text.select(CaretRange(6, 14));
      render_text(dc,
        text,
        no_option(),
        tri_from_rect(Rect(Point(0,0), Size(200, 100))),
        beingEdited,
        textInfo,
        ctx,
        s);
    }
    save_test_image(bmp, FileName("render-text-selection-basic.png"));
  }

  {
    StubExpressionContext ctx;
    auto render_selected =
      [&](TextBuffer& text, const CaretRange& r){
      text.select(r);
      Bitmap bmp({200, 200}, color_white);
      {
        FaintDC dc(bmp);
        render_text(dc,
          text,
          no_option(),
          tri_from_rect(Rect(Point(0,0), Size(200, 100))),
          beingEdited,
          textInfo,
          ctx,
          s);
      }
      FileName f(no_sep("render-text-selection-",
          str_uint(r.from), "-to-", str_uint(r.to), ".png"));
      save_test_image(bmp, f);
    };

    // ".a.b.c. ."
    // "0 1 2 3 4"

    TextBuffer text("abc\ndef");
    for (size_t i = 0; i != text.size(); i++){
      render_selected(text, CaretRange(0, i));
    }
  }

  {
    // Fixme: Previously crashed in compute_caret.
    // Extract to compute_caret test instead.
    Bitmap bmp({200, 200}, color_white);
    {
      FaintDC dc(bmp);

      StubExpressionContext ctx;
      TextBuffer text;
      text.insert(utf8_char("a"));
      text.insert(chars::eol);

      render_text(dc,
        text,
        no_option(),
        tri_from_rect(Rect(Point(0,0), Size(200, 100))),
        beingEdited,
        textInfo,
        ctx,
        s);
    }
    save_test_image(bmp, FileName("wee.png"));
  }
}
