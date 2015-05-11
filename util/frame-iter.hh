// -*- coding: us-ascii-unix -*-
// Copyright 2015 Lukas Kemmer
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

#ifndef FAINT_FRAME_ITER_HH
#define FAINT_FRAME_ITER_HH
#include "app/canvas.hh"
#include "util/index-iter.hh"

namespace faint{

class FrameIter{
public:
  using value_type = const Image;
  using iterator_category = std::input_iterator_tag;
  using difference_type = size_t;
  using pointer = const Image*;
  using reference = const Image&;
  FrameIter(const Canvas& canvas, const Index& index)
    : m_canvas(canvas),
      m_iter(index)
  {}

  const Image* operator->() const{
    return &(m_canvas.GetFrame(*m_iter));
  }

  const Image& operator*() const{
    return m_canvas.GetFrame(*m_iter);
  }

  bool operator!=(const FrameIter& other) const{
    return m_iter != other.m_iter;
  }

  bool operator==(const FrameIter& other) const{
    return m_iter == other.m_iter;
  }

  FrameIter& operator++(){
    ++m_iter;
    return *this;
  }

  FrameIter& operator=(const FrameIter&) = delete;
private:
  const Canvas& m_canvas;
  IndexIter m_iter;
};

inline FrameIter begin(const Canvas& c){
  return FrameIter(c, 0_idx);
}

inline FrameIter end(const Canvas& c){
  return FrameIter(c, c.GetNumFrames());
}

} // namespace

#endif
