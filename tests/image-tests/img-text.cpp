// -*- coding: us-ascii-unix -*-
#include "test-sys/test-name.hh"
#include "tests/test-util/file-handling.hh"
#include "tests/test-util/image-table.hh"

#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "geo/tri.hh"
#include "rendering/faint-dc.hh"
#include "text/char-constants.hh"
#include "text/formatting.hh"
#include "util/default-settings.hh"
#include "util/setting-id.hh"

static faint::Bitmap with_rect(const faint::Bitmap& src, const faint::Tri& r){
  faint::Bitmap b2(src);
  faint::FaintDC dc(b2);
  dc.Rectangle(r, faint::default_rectangle_settings());
  return b2;
}

void img_text(){
  // Test FaintDC::Text

  using namespace faint;
  ImageTable t(get_test_name(), {"", "Text", "Given bounds"});
  const utf8_string text = "J" + chars::latin_small_letter_o_with_diaresis +
    "rpsylta";

  std::vector<std::pair<utf8_string, Settings> > settings;

  Settings s1(default_text_settings());
  settings.emplace_back("Default", s1);
  Settings s2(s1);
  s2.Set(ts_FontSize, s2.Get(ts_FontSize) * 2);
  settings.emplace_back("Larger", s2);

  Settings s3(s2);
  s3.Set(ts_FontBold, true);
  settings.emplace_back("Bold", s3);

  Settings s4(s2);
  s4.Set(ts_FontItalic, true);
  settings.emplace_back("Italic", s4);

  int imageNum = 0;
  for (const auto& p : settings){
    const auto& label = p.first;
    const auto& s = p.second;
    Bitmap bmp({200,50}, grayscale_rgba(200));
    FaintDC dc(bmp);
    Tri bounds({10,10},{190,10},{10,40});
    dc.Text(bounds, text, s);

    t.AddRow(label,
      save_test_image(bmp,
        FileName("text_" + str_int(imageNum, left_pad(3)) + ".png")),

      save_test_image(with_rect(bmp, bounds),
        FileName("text_" + str_int(imageNum + 1, left_pad(3)) + ".png")));
    imageNum += 2;
  }

  save_image_table(t);
}
