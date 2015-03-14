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

#include "app/canvas-handle.hh"
#include "geo/int-rect.hh"
#include "util/iter.hh"

namespace faint{

const IntSize g_canvasHandleSz(6,6);

HandlePos opposite_handle_pos(HandlePos pos){
  switch (pos){
  case HandlePos::TOP_LEFT: return HandlePos::BOTTOM_RIGHT;
  case HandlePos::TOP_RIGHT: return HandlePos::BOTTOM_LEFT;
  case HandlePos::BOTTOM_LEFT: return HandlePos::TOP_RIGHT;
  case HandlePos::BOTTOM_RIGHT: return HandlePos::TOP_LEFT;
  case HandlePos::TOP_SIDE: return HandlePos::BOTTOM_SIDE;
  case HandlePos::BOTTOM_SIDE: return HandlePos::TOP_SIDE;
  case HandlePos::LEFT_SIDE: return HandlePos::RIGHT_SIDE;
  case HandlePos::RIGHT_SIDE: return HandlePos::LEFT_SIDE;
  case HandlePos::END: assert(false); return HandlePos::LEFT_SIDE;
  };
  assert(false);
  return HandlePos::BEGIN;
}

HandleDirection handle_direction(HandlePos pos){
  if (pos <= HandlePos::BOTTOM_RIGHT){
    return HandleDirection::DIAGONAL;
  }
  else if (pos == HandlePos::TOP_SIDE){
    return HandleDirection::UP_DOWN;
  }
  else if (pos == HandlePos::BOTTOM_SIDE){
    return HandleDirection::UP_DOWN;
  }
  else if (pos == HandlePos::LEFT_SIDE){
    return HandleDirection::LEFT_RIGHT;
  }
  else if (pos == HandlePos::RIGHT_SIDE){
    return HandleDirection::LEFT_RIGHT;
  }
  assert(false);
  return HandleDirection::DIAGONAL;
}

CanvasResizeHandle::CanvasResizeHandle(HandlePos handlePos,
  const IntSize& canvasSize,
  const CanvasGeo& geo)
  : m_canvasSize(canvasSize),
    m_geo(geo),
    m_handlePos(handlePos)
{}

IntSize CanvasResizeHandle::GetCanvasSize() const{
  return m_canvasSize;
}

Cursor CanvasResizeHandle::GetCursor() const{
  using HP = HandlePos;
  if (m_handlePos == HP::TOP_LEFT || m_handlePos == HP::BOTTOM_RIGHT){
    return Cursor::RESIZE_NW;
  }
  else if (m_handlePos == HP::TOP_RIGHT || m_handlePos == HP::BOTTOM_LEFT){
    return Cursor::RESIZE_NE;
  }
  else if (m_handlePos == HP::LEFT_SIDE || m_handlePos == HP::RIGHT_SIDE){
    return Cursor::RESIZE_WE;
  }
  else if (m_handlePos == HP::TOP_SIDE || m_handlePos == HP::BOTTOM_SIDE){
    return Cursor::RESIZE_NS;
  }
  assert(false);
  return Cursor::RESIZE_NW;
}

HandleDirection CanvasResizeHandle::GetDirection() const{
  return handle_direction(m_handlePos);
}

IntPoint CanvasResizeHandle::GetPos() const{
  switch (m_handlePos){
  case HandlePos::TOP_LEFT:
    return IntPoint(0,0);
  case HandlePos::TOP_RIGHT:
    return IntPoint(m_canvasSize.w - 1, 0);
  case HandlePos::BOTTOM_LEFT:
    return IntPoint(0, m_canvasSize.h - 1);
  case HandlePos::BOTTOM_RIGHT:
    return IntPoint(m_canvasSize.w - 1, m_canvasSize.h - 1);
  case HandlePos::TOP_SIDE:
    return IntPoint((m_canvasSize.w - 1)/2, 0);
  case HandlePos::BOTTOM_SIDE:
    return IntPoint((m_canvasSize.w - 1)/2, m_canvasSize.h - 1);
  case HandlePos::LEFT_SIDE:
    return IntPoint(0, (m_canvasSize.h - 1)/2);
  case HandlePos::RIGHT_SIDE:
    return IntPoint(m_canvasSize.w - 1, (m_canvasSize.h - 1)/2);
  case HandlePos::END:
    assert(false);
    return IntPoint(0,0);
  }
  assert(false);
  return IntPoint(0,0);
}

IntRect CanvasResizeHandle::GetRect() const{
  const int xOffset = m_geo.border.w / 2 - g_canvasHandleSz.w / 2;
  const int yOffset = m_geo.border.h / 2 - g_canvasHandleSz.h / 2;

  const int left = -m_geo.pos.x + xOffset;
  const int top = -m_geo.pos.y + yOffset;

  const coord zoom = m_geo.zoom.GetScaleFactor();
  const int width = rounded(m_canvasSize.w * zoom);
  const int height = rounded(m_canvasSize.h * zoom);
  const int right = m_geo.border.w + width - m_geo.pos.x + xOffset;
  const int bottom = m_geo.border.h + height - m_geo.pos.y + yOffset;

  if (m_handlePos == HandlePos::TOP_LEFT){
    return IntRect(IntPoint(left,top), g_canvasHandleSz);
  }
  else if (m_handlePos == HandlePos::TOP_RIGHT){
    return IntRect(IntPoint(right, top), g_canvasHandleSz);
  }
  else if (m_handlePos == HandlePos::BOTTOM_LEFT){
    return IntRect(IntPoint(left, bottom), g_canvasHandleSz);
  }
  else if (m_handlePos == HandlePos::BOTTOM_RIGHT){
    return IntRect(IntPoint(right, bottom), g_canvasHandleSz);
  }
  else if (m_handlePos == HandlePos::TOP_SIDE){
    const int ch_w = g_canvasHandleSz.w;
    return IntRect(IntPoint((int)m_geo.border.w + width / 2 - m_geo.pos.x - ch_w / 2, top),
      g_canvasHandleSz);
  }
  else if (m_handlePos == HandlePos::BOTTOM_SIDE){
    const int ch_w = g_canvasHandleSz.w;
    return IntRect(IntPoint(m_geo.border.w +
        width / 2 - m_geo.pos.x - ch_w / 2, bottom),
      g_canvasHandleSz);
  }
  else if (m_handlePos == HandlePos::RIGHT_SIDE){
    const int ch_h = g_canvasHandleSz.h;
    return IntRect(IntPoint(right, m_geo.border.h +
        height / 2 - m_geo.pos.y - ch_h / 2),
      g_canvasHandleSz);
  }
  else if (m_handlePos == HandlePos::LEFT_SIDE){
    const int ch_h = g_canvasHandleSz.h;
    return IntRect(IntPoint(left,m_geo.border.h +
        height / 2 - m_geo.pos.y - ch_h / 2),
      g_canvasHandleSz);
  }
  else{
    assert(false);
    return IntRect();
  }
}

CanvasResizeHandle CanvasResizeHandle::Opposite() const{
  return CanvasResizeHandle(opposite_handle_pos(m_handlePos), m_canvasSize,
    m_geo);
}

Optional<CanvasResizeHandle> canvas_handle_hit_test(const IntPoint& mousePos,
  const IntSize& canvasSize, const CanvasGeo& geo)
{
  for (const HandlePos& hp : iterable<HandlePos>()){
    CanvasResizeHandle handle(hp, canvasSize, geo);
    if (handle.GetRect().Contains(mousePos)){
      return option(handle);
    }
  }
  return no_option();
}

} // namespace
