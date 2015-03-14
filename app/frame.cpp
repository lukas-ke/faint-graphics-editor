// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
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

#include "app/canvas.hh"
#include "app/frame.hh"
#include "util/image.hh"
#include "util/index.hh"

namespace faint{

Frame::Frame(Canvas* canvas, const Image& image)
  : canvas(canvas),
    image(image),
    frameId(image.GetId())
{}

const Either<Bitmap, ColorSpan>& Frame::GetBackground() const{
  return image.GetBackground();
}

Index Frame::GetFrameIndex() const{
  return canvas->GetFrameIndex(image);
}

const Image& Frame::GetImage() const{
  return image;
}

const RasterSelection& Frame::GetRasterSelection() const{
  return image.GetRasterSelection();
}

bool contains_pos(const Frame& frame, const IntPoint& pos){
  if (!fully_positive(pos)){
    return false;
  }

  IntSize size = frame.GetImage().GetSize();
  return pos.x < size.w && pos.y < size.h;
}

} // namespace
