// -*- coding: us-ascii-unix -*-
// Copyright 2020 Lukas Kemmer
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

#include "webp/decode.h"

#include "formats/format-errors-common.hh"
#include "formats/webp/file-webp.hh"
#include "util-wx/stream.hh" // BinaryReader

namespace faint{

OrError<Bitmap> read_webp(const FilePath& p){
  BinaryReader in(p);
  if (!in.good()){
    return error_open_file_read(p);
  }

  std::vector<unsigned char> v = read_into_vector(p);
  int w, h;
  const auto ok = WebPGetInfo(v.data(), v.size(), &w, &h);
  if (!ok) {
    return "Failed reading info header.";
  }

  WebPBitstreamFeatures features;
  const auto status = WebPGetFeatures(v.data(), v.size(), &features);
  if (status != VP8_STATUS_OK){
    // FIXME: Include error code
    return "Failed reading info header.";
  }

  Bitmap bmp(IntSize(w, h));
  auto bmpData = WebPDecodeBGRAInto(v.data(), v.size(), bmp.GetRaw(), bmp.GetRawSize(), bmp.GetStride());
  if (bmpData == nullptr) {
    return "Failed decoding BGRA data";
  }

  return bmp;
}

} // namespace
