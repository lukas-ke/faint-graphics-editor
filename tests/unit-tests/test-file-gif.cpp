// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/bitmap-test-util.hh"
#include "tests/test-util/file-handling.hh"
#include "tests/test-util/print-objects.hh"
#include "bitmap/draw.hh"
#include "formats/gif/file-gif.hh"
#include "util/image-props.hh"

namespace{

// Fixme: Pass delay too!
void check_frame(const faint::FrameProps& p, const faint::Bitmap& key){
  using namespace faint;
  return p.GetBackground().Visit(
    [&](const Bitmap& bmp){
      EQUAL(bmp.GetSize(), key.GetSize());
      // FIXME: Why ignore transparency?
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

std::ostream& operator<<(std::ostream& o, const faint::Delay& delay){
  // Printer for EQUAL-macro
  return o << delay.Get();
}

} // namespace

void test_file_gif(){
  using namespace faint;

  std::vector<Bitmap> keys = {load_key("86-68-key-1.png"),
                              load_key("86-68-key-2.png"),
                              load_key("86-68-key-3.png"),
                              load_key("86-68-key-4.png")};

  {
    // Read 86-68.gif, and check the frames frames against the
    // comparison png images
    ImageProps props;
    read_gif(get_test_load_path(FileName("86-68.gif")), props);
    ABORT_IF(props.GetNumFrames() != 4);
    FWD(check_frame(props.GetFrame(0_idx), keys[0]));
    FWD(check_frame(props.GetFrame(1_idx), keys[1]));
    FWD(check_frame(props.GetFrame(2_idx), keys[2]));
    FWD(check_frame(props.GetFrame(3_idx), keys[3]));
  }

  {
    // Write a single frame
    auto img = load_test_image(FileName("gauss-source.png"));
    auto mapped = quantized(img, Dithering::ON);
    std::vector<MappedColors_and_delay> images = {{mapped, Delay(0)}};
    auto result = write_gif(get_test_save_path(FileName("libgif.gif")), images);
    if (!result.Successful()){
      FAIL(result.ErrorDescription().c_str());
    }
    // Fixme: Reload and check it.
  }

  {
    // Read/write a gif, write it out with new delays
    // reload it, and check the delays and content.
    ImageProps srcProps;
    read_gif(get_test_load_path(FileName("86-68.gif")), srcProps);
    ABORT_IF(srcProps.GetNumFrames() != 4);

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

    auto delays = std::vector<Delay>({{10}, {20}, {30}, {40}});

    std::vector<MappedColors_and_delay> images = {
      {get_bmp(srcProps.GetFrame(0_idx)), delays[0]},
      {get_bmp(srcProps.GetFrame(1_idx)), delays[1]},
      {get_bmp(srcProps.GetFrame(2_idx)), delays[2]},
      {get_bmp(srcProps.GetFrame(3_idx)), delays[3]}};

    const auto outPath = get_test_save_path(FileName("libgif86-68.gif"));
    auto result = write_gif(outPath, images);
    if (!result.Successful()){
      FAIL(result.ErrorDescription().c_str());
    }

    // Re-read the saved gif
    ImageProps props;
    read_gif(outPath, props);
    ABORT_IF(props.GetNumFrames() != 4);
    for (int i = 0; i != props.GetNumFrames().Get(); i++){
      const auto& frame = props.GetFrame(Index(i));
      EQUAL(frame.GetDelay(), delays[i]);
      // FWD(check_frame(frame, keys[0])); <- Fixme: Fails, because write gif loses transparency

    }
  }
}
