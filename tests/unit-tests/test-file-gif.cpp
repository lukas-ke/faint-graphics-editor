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
  // instead of the saved 255,255,255,0 // Fixme: Still true now that I use libpng?
  replace_color_color(bmp, Old(color_transparent_black),
    New(color_transparent_white));
  return bmp;
}

} // namespace

void test_file_gif(){
  using namespace faint;
  {
    // Read
    ImageProps props;
    read_gif(get_test_load_path(FileName("86-68.gif")), props);
    ABORT_IF(props.GetNumFrames() != 4);
    FWD(check_frame(props.GetFrame(0_idx), load_key("86-68-key-1.png")));
    FWD(check_frame(props.GetFrame(1_idx), load_key("86-68-key-2.png")));
    FWD(check_frame(props.GetFrame(2_idx), load_key("86-68-key-3.png")));
    FWD(check_frame(props.GetFrame(3_idx), load_key("86-68-key-4.png")));
  }

  {
    // Write
    auto img = load_test_image(FileName("gauss-source.png"));
    auto mapped = quantized(img, Dithering::ON);
    std::vector<MappedColors_and_delay> images = {{mapped, Delay(0)}};
    auto result = write_gif(get_test_save_path(FileName("libgif.gif")), images);
    if (!result.Successful()){
      MESSAGE(result.ErrorDescription().c_str());
      FAIL();
    }
    // Fixme: Reload and check it.
  }

  {
    // Read/write
    ImageProps props;
    read_gif(get_test_load_path(FileName("86-68.gif")), props);
    ABORT_IF(props.GetNumFrames() != 4);

    auto get_bmp =
      [](const FrameProps& p){
        return p.GetBackground().Visit(
          [](const Bitmap& bmp) -> MappedColors{
            return quantized(bmp, Dithering::ON);
          },
          [](const ColorSpan&) -> MappedColors{
            FAIL("Got color span instead of Bitmap.");
          });
      };

    std::vector<MappedColors_and_delay> images = {
      {get_bmp(props.GetFrame(0_idx)), Delay(0)},
      {get_bmp(props.GetFrame(1_idx)), Delay(0)},
      {get_bmp(props.GetFrame(2_idx)), Delay(0)},
      {get_bmp(props.GetFrame(3_idx)), Delay(0)}};
    auto result = write_gif(get_test_save_path(FileName("libgif86-68.gif")),
      images);
    if (!result.Successful()){
      MESSAGE(result.ErrorDescription().c_str());
      FAIL();
    }
  }

}
