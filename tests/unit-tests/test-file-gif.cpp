// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/bitmap-test-util.hh"
#include "tests/test-util/file-handling.hh"
#include "tests/test-util/print-objects.hh"
#include "bitmap/draw.hh"
#include "formats/gif/file-gif.hh"
#include "util/image-props.hh"

namespace{

void check_frame(const faint::FrameProps& p, const faint::Bitmap& key){
  using namespace faint;
  return p.GetBackground().Visit(
    [&](const Bitmap& bmp){
      EQUAL(bmp.GetSize(), key.GetSize());
      VERIFY(equal_ignore_transparent(bmp, key));
    },
    [](const ColorSpan&){
      FAIL("Got color span instead of Bitmap");
    });
}

faint::Bitmap load_key(const char* filename){
  using namespace faint;
  auto bmp = load_test_image(faint::FileName(filename));

  // Unfortunately, the png loading of the keys replaces the alpha
  // transparency with a mask, which when undone gets color 0,0,0,0
  // instead of the saved 255,255,255,0
  replace_color_color(bmp, Old(color_transparent_black),
    New(color_transparent_white));
  return bmp;
}

} // namespace

void test_file_gif(){
  using namespace faint;
  ImageProps props;
  read_gif(get_test_load_path(FileName("86-68.gif")), props);
  ABORT_IF(props.GetNumFrames() != 4);
  FWD(check_frame(props.GetFrame(0_idx), load_key("86-68-key-1.png")));
  FWD(check_frame(props.GetFrame(1_idx), load_key("86-68-key-2.png")));
  FWD(check_frame(props.GetFrame(2_idx), load_key("86-68-key-3.png")));
  FWD(check_frame(props.GetFrame(3_idx), load_key("86-68-key-4.png")));
}
