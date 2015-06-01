// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/bitmap-test-util.hh"
#include "tests/test-util/file-handling.hh"
#include "tests/test-util/print-objects.hh"
#include "bitmap/draw.hh"
#include "formats/gif/file-gif.hh"
#include "util/image-props.hh"
#include "util/iter.hh"

namespace{

void check_frame(const faint::FrameProps& p,
  const faint::Bitmap& key,
  const faint::Delay& expectedDelay)
{
  using namespace faint;

  EQUAL(p.GetDelay().Get(), expectedDelay.Get());

  return p.GetBackground().Visit(
    [&](const Bitmap& bmp){
      EQUAL(bmp.GetSize(), key.GetSize());
      VERIFY(equal(bmp, key));
    },
    [](const ColorSpan&){
      FAIL("Got color span instead of Bitmap");
    });
}

faint::Bitmap load_key(const char* filename){
  using namespace faint;
  return load_test_image(faint::FileName(filename));
}

} // namespace
void test_file_gif(){
  using namespace faint;

  std::vector<Bitmap> keys = {load_key("86-68-key-1.png"),
                              load_key("86-68-key-2.png"),
                              load_key("86-68-key-3.png"),
                              load_key("86-68-key-4.png")};

  EQUAL(get_color(keys[0], {0, 0}), Color(255,255,255,0));

  {
    // Read 86-68.gif, and check the frames frames against the
    // comparison png images
    ImageProps props;
    read_gif(get_test_load_path(FileName("86-68.gif")), props);
    ABORT_IF(props.GetNumFrames() != 4);
    FWD(check_frame(props.GetFrame(0_idx), keys[0], Delay(10)));
    FWD(check_frame(props.GetFrame(1_idx), keys[1], Delay(10)));
    FWD(check_frame(props.GetFrame(2_idx), keys[2], Delay(10)));
    FWD(check_frame(props.GetFrame(3_idx), keys[3], Delay(10)));
  }

  {
    // Write a single frame
    auto img = load_test_image(FileName("gauss-source.png"));
    auto mapped = quantized(img, Dithering::ON);
    std::vector<GifFrame> images = {{mapped, Delay(0)}};

    auto filename = suffix_u8_chars(FileName("single-frame.gif"));
    auto savePath = get_test_save_path(filename);
    auto result = write_gif(savePath, images);
    if (!result.Successful()){
      FAIL(result.ErrorDescription().c_str());
    }

    // Reload and check with against gauss-dithered.png. This doesn't
    // imply that gauss-dithered.png is perfect, it just intends to
    // catch inadvertent changes.
    auto key = load_test_image(FileName("gauss-dithered.png"));
    ImageProps props;
    read_gif(savePath, props);
    ABORT_IF(props.GetNumFrames() != 1);
    FWD(check_frame(props.GetFrame(0_idx), key, Delay(0)));
  }

  {
    // Write a single frame with transparency
    auto src = load_test_image(FileName("86-68-key-1.png"));
    auto mapped = quantized(src, Dithering::ON);
    std::vector<GifFrame> images = {{mapped, Delay(0)}};

    auto outFile = suffix_u8_chars(FileName("single-frame-transparency.gif"));
    auto savePath = get_test_save_path(outFile);

    auto result = write_gif(savePath, images);
    if (!result.Successful()){
      FAIL(result.ErrorDescription().c_str());
    }

    // Reload and check
    ImageProps props;
    read_gif(savePath, props);
    ABORT_IF(props.GetNumFrames() != 1);
    FWD(check_frame(props.GetFrame(0_idx), src, Delay(0)));
  }

  {
    // Read/write a gif, write it out with new delays reload it, and
    // check the delays and content.
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

    std::vector<GifFrame> images = {
      {get_bmp(srcProps.GetFrame(0_idx)), delays[0]},
      {get_bmp(srcProps.GetFrame(1_idx)), delays[1]},
      {get_bmp(srcProps.GetFrame(2_idx)), delays[2]},
      {get_bmp(srcProps.GetFrame(3_idx)), delays[3]}};

    VERIFY(images[0].image.transparencyIndex.IsSet());

    const auto outPath = get_test_save_path(FileName("libgif86-68.gif"));
    auto result = write_gif(outPath, images);
    if (!result.Successful()){
      FAIL(result.ErrorDescription().c_str());
    }

    // Re-read and verify the saved gif
    ImageProps props;
    read_gif(outPath, props);
    ABORT_IF(props.GetNumFrames() != 4);
    for (int i = 0; i != props.GetNumFrames().Get(); i++){
      const auto& frame = props.GetFrame(Index(i));
      FWD(check_frame(frame, keys[i], delays[i]));
    }
  }
}
