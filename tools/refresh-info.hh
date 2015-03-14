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

#ifndef FAINT_REFRESH_INFO
#define FAINT_REFRESH_INFO
#include "geo/int-rect.hh"
#include "geo/point.hh"

namespace faint{

class RefreshInfo{
  // A collection of parameters needed by some tools for determining
  // the image rectangle that needs to be refreshed.
public:
  RefreshInfo(const IntRect& visibleRect, Point mousePos, int objectHandleWidth)
    : visibleRect(visibleRect),
      mousePos(mousePos),
      objectHandleWidth(objectHandleWidth)
  {}

  // The rectangle of pixels that are at least partially visible in
  // the UI (considering zoom, window size etc.).
  IntRect visibleRect;

  // The mouse position in image sub-pixel-coordinates
  Point mousePos;

  // The width of resize handle overlays for objects
  int objectHandleWidth;
};

} // namespace

#endif
