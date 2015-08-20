// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/file-handling.hh"
#include "rendering/render-text.hh"
#include "util/default-settings.hh"
#include "objects/objtext.hh" // Fixme: Move TextInfoDC elsewhere.
#include "rendering/text-info-dc.hh"
#include "geo/tri.hh"
#include "geo/rect.hh"
#include "geo/size.hh"

namespace faint{

class StubExpressionContext : public ExpressionContext{
  Optional<Calibration> GetCalibration() const override{
    return no_option();
  }

  const Object* GetObject(const utf8_string&) const override{
    return nullptr;
  }
};

} // namespace

void img_render_text(){
  using namespace faint;

  TextBuffer text("Hello world\n");

  auto s = default_text_settings();
  TextInfoDC textInfo(s);
  Bitmap bmp({200, 200});

  {
    FaintDC dc(bmp);

    StubExpressionContext ctx;

    render_text(dc,
      text,
      no_option(),
      tri_from_rect(Rect(Point(0,0), Size(100, 100))),
      false,
      textInfo,
      ctx,
      s);
  }
  save_test_image(bmp, FileName("render-text.png"));
}
