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

#ifndef FAINT_IMAGE_PROPS_HH
#define FAINT_IMAGE_PROPS_HH
#include "text/utf8-string.hh"
#include "util/frame-props.hh"
#include "util/index.hh"

namespace faint{

class ImageProps{
  // For creating an image, e.g. during loading.
public:
  ImageProps();
  ImageProps(ImageProps&&);
  explicit ImageProps(const ImageInfo& firstFrame);
  explicit ImageProps(const Bitmap& firstBmp);
  ImageProps(const IntSize&, const objects_t&);
  FrameProps& AddFrame(const ImageInfo&);
  FrameProps& AddFrame(Bitmap&&, const FrameInfo&);
  void AddWarning(const utf8_string&);
  utf8_string GetError() const;
  FrameProps& GetFrame(const Index&);
  Index GetNumFrames() const;
  int GetNumWarnings() const;
  utf8_string GetWarning(int) const;
  bool IsOk() const;
  void SetError(const utf8_string&);
private:
  utf8_string m_error;
  std::vector<FrameProps> m_frames;
  bool m_ok;
  std::vector<utf8_string> m_warnings;
};

} // namespace

#endif
