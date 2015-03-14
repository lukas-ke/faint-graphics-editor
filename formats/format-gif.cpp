// -*- coding: us-ascii-unix -*-
// Copyright 2012 Lukas Kemmer
//
// Licensed under the Apache License, Version 2.0 (the "License"); you
// may not use this file except in compliance with the License. You
// may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied. See the License for the specific language governing
// permissions and limitations under the License.

#include <sstream>
#include "wx/anidecod.h" // wxImageArray
#include "wx/quantize.h"
#include "wx/wfstream.h"
#include "app/canvas.hh"
#include "formats/format.hh"
#include "formats/gif/file-gif.hh"
#include "text/formatting.hh"
#include "util/image.hh"
#include "util/image-util.hh"
#include "util/index-iter.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/scoped-error-log.hh"
#include "util/make-vector.hh"

namespace faint{

static size_t find_mismatch_index(const std::vector<IntSize>& sizes){
  assert(!sizes.empty());
  size_t i = 1;
  for (; i != sizes.size(); i++){
    if (sizes[i] != sizes[0]){
      break;
    }
  }
  return i;
}

static bool uniform_size(const std::vector<IntSize>& sizes){
  return find_mismatch_index(sizes) == sizes.size();
}

static SaveResult fail_size_mismatch(const std::vector<IntSize>& sizes){
  size_t index = find_mismatch_index(sizes);
  assert(index != sizes.size());
  std::stringstream ss;
  ss << "This image can not be saved as a gif." << std::endl << std::endl <<
    "It contains frames of different sizes." << std::endl <<
    "Frame 1: " << str(sizes[0]) << std::endl <<
    "Frame " << index + 1 << ": " << str(sizes[index]);
  return SaveResult::SaveFailed(utf8_string(ss.str()));
}

static SaveResult fail_stream_error(const FilePath& filePath){
  return SaveResult::SaveFailed(space_sep("Faint could not write to",
    quoted(filePath.Str()) + "."));
}

static std::vector<IntSize> get_frame_sizes(Canvas& canvas){
  return make_vector(up_to(canvas.GetNumFrames()),
    [&](const auto& i){
      return canvas.GetFrame(i).GetSize();
    });
}

static SaveResult fail_save(const FilePath& filePath, const ScopedErrorLog& log){
  return SaveResult::SaveFailed(endline_sep(space_sep(
    "Failed saving to", quoted(filePath.Str()) + ".\n"),
    log.GetMessages()));
}

class FormatGIF : public Format {
public:
  FormatGIF()
    : Format(FileExtension("gif"),
      label_t(utf8_string("Graphics Interchange Format (GIF)")),
      can_save(true),
      can_load(true))
  {}

  void Load(const FilePath& filePath, ImageProps& imageProps) override{
    read_gif(filePath, imageProps);
  }

  SaveResult Save(const FilePath& filePath, Canvas& canvas) override{
    std::vector<IntSize> sizes = get_frame_sizes(canvas);
    if (!uniform_size(sizes)){
      return fail_size_mismatch(sizes);
    }

    wxImageArray images;
    for (auto i : up_to(canvas.GetNumFrames())){
      Bitmap bmp(flatten(canvas.GetFrame(i)));
      wxImage img(to_wx_image(bmp));
      wxQuantize::Quantize(img, img); // Fixme: Need same quantization for every frame
      images.Add(new wxImage(img)); // wxImageArray uses delete
    }

    int delay_ms = canvas.GetFrame(0_idx).GetDelay().Get(); // Fixme: Should have per-frame delay

    wxFileOutputStream out(to_wx(filePath.Str()));
    if (!out.IsOk()){
      return fail_stream_error(filePath);
    }

    // Collect wx log messages
    ScopedErrorLog logger;

    wxGIFHandler handler;
    bool ok = handler.SaveAnimation(images, &out, false, delay_ms);
    if (ok){
      return SaveResult::SaveSuccessful();
    }
    return fail_save(filePath, logger);
  }
};

Format* format_gif(){
  return new FormatGIF();
}

} // namespace
