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

#include "util/image-props.hh"

namespace faint{

ImageProps::ImageProps()
  : m_ok(true)
{}

ImageProps::ImageProps(ImageProps&& other)
  : m_error(std::move(other.m_error)),
    m_frames(std::move(other.m_frames)),
    m_ok(other.m_ok),
    m_warnings(other.m_warnings)
{}

ImageProps::ImageProps(const ImageInfo& firstFrame)
  : m_ok(true)
{
  m_frames.emplace_back(firstFrame);
}

ImageProps::ImageProps(const Bitmap& firstBmp)
  : m_ok(true)
{
  m_frames.emplace_back(firstBmp);
}

ImageProps::ImageProps(const IntSize& size, const objects_t& objects)
  : m_ok(true)
{
  m_frames.emplace_back(size, objects);
}

FrameProps& ImageProps::AddFrame(const ImageInfo& info){
  m_frames.emplace_back(info);
  return m_frames.back();
}

FrameProps& ImageProps::AddFrame(Bitmap&& bmp, const FrameInfo& info){
  m_frames.emplace_back(std::move(bmp), info);
  return m_frames.back();
}

FrameProps& ImageProps::GetFrame(const Index& index){
  assert(valid_index(index, m_frames));
  return m_frames[to_size_t(index.Get())];
}

Index ImageProps::GetNumFrames() const{
  return to_index(m_frames.size());
}

int ImageProps::GetNumWarnings() const{
  return resigned(m_warnings.size());
}

utf8_string ImageProps::GetWarning(int num) const{
  assert(to_size_t(num) < m_warnings.size());
  return m_warnings[to_size_t(num)];
}

utf8_string ImageProps::GetError() const{
  if (m_ok && m_frames.empty()){
    // No error was set by the format, yet no frame was created.
    return "No image frame created during loading.";
  }
  assert(!m_ok);
  return m_error;
}

bool ImageProps::IsOk() const{
  return m_ok && !m_frames.empty();
}

void ImageProps::SetError(const utf8_string& error){
  m_error = error;
  m_ok = false;
}

void ImageProps::AddWarning(const utf8_string& warning){
  m_warnings.push_back(warning);
}

} // namespace
