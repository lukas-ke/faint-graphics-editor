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

#ifndef FAINT_CANVAS_HANDLE_HH
#define FAINT_CANVAS_HANDLE_HH
#include "app/resource-id.hh" // For Cursor
#include "geo/canvas-geo.hh"
#include "geo/int-point.hh"
#include "geo/int-rect.hh"
#include "geo/int-size.hh"

namespace faint{

enum class HandlePos{
  BEGIN = 0,
  TOP_LEFT = BEGIN,
  TOP_RIGHT = 1,
  BOTTOM_LEFT = 2,
  BOTTOM_RIGHT = 3,
  TOP_SIDE = 4,
  BOTTOM_SIDE = 5,
  LEFT_SIDE = 6,
  RIGHT_SIDE = 7,
  END
};

enum class HandleDirection{
  UP_DOWN,
  LEFT_RIGHT,
  DIAGONAL
};

HandlePos opposite_handle_pos(HandlePos);
HandleDirection handle_direction(HandlePos);

class CanvasResizeHandle{
public:
  CanvasResizeHandle(HandlePos, const IntSize& canvasSize, const CanvasGeo&);
  IntSize GetCanvasSize() const;
  Cursor GetCursor() const;
  HandleDirection GetDirection() const;
  IntPoint GetPos() const;
  IntRect GetRect() const;
  CanvasResizeHandle Opposite() const;
private:
  IntSize m_canvasSize;
  CanvasGeo m_geo;
  HandlePos m_handlePos;
};

Optional<CanvasResizeHandle> canvas_handle_hit_test(const IntPoint&,
  const IntSize& canvasSize,
  const CanvasGeo&);

} // namespace

#endif
