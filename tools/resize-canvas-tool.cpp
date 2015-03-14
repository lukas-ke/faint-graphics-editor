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

#include <cassert>
#include "app/canvas.hh"
#include "app/canvas-handle.hh"
#include "commands/rescale-cmd.hh"
#include "geo/adjust.hh"
#include "geo/geo-func.hh"
#include "geo/rect.hh"
#include "rendering/overlay.hh"
#include "text/formatting.hh"
#include "tools/resize-canvas-tool.hh"
#include "tools/standard-tool.hh"
#include "util/command-util.hh"
#include "util/default-settings.hh"
#include "util/image-util.hh"
#include "util/pos-info.hh"

namespace faint{

static bool can_constrain(HandleDirection dir){
  return dir == HandleDirection::DIAGONAL;
}

static bool constrain_resize(const ToolModifiers& mod, HandleDirection dir){
  return mod.Secondary() && can_constrain(dir);
}

static IntPoint get_x2y2(const CanvasResizeHandle& handle,
  const IntPoint& release)
{
  // The rectangle to resize to is determined by the handle release position
  // and the opposite handle position.
  //
  // Since the image boundary is on the edge of a pixel, that pixel
  // should only be included in the resize rectangle if the release
  // point has not wrapped over the opposite handle.

  // Example: L=Left handle, R=Right handle, x = "handle pixel",
  // Include the first pixel (no wrap)):
  // 1)   Lx.R
  // 2)   Lx...R
  //
  // Do not include the first pixel (the right handle has wrapped over
  // the left handle):
  // 3)    Lx.R
  // 4) R..L

  IntPoint handlePos = handle.GetPos();
  IntPoint opposite = handle.Opposite().GetPos();
  int x2 = opposite.x;
  int y2 = opposite.y;
  if (handlePos.x > opposite.x){
    if (release.x < opposite.x){
      x2 -= 1;
    }
  }
  else {
    if (release.x > opposite.x){
      x2 += 1;
    }
  }

  if (handlePos.y > opposite.y){
    if (release.y < opposite.y){
      y2 -= 1;
    }
  }
  else {
    if (release.y > opposite.y){
      y2 += 1;
    }
  }
  return IntPoint(x2, y2);
}

static IntRect get_resize_rect(const CanvasResizeHandle& handle,
  const IntPoint& release)
{
  HandleDirection dir = handle.GetDirection();
  if (dir == HandleDirection::DIAGONAL){
    return IntRect(get_x2y2(handle, release), release);
  }
  else if (dir == HandleDirection::UP_DOWN){
    int x2 = handle.GetCanvasSize().w - 1;
    int y2 = get_x2y2(handle, release).y;
    return IntRect(IntPoint(0, release.y), IntPoint(x2, y2));
  }
  else if (dir == HandleDirection::LEFT_RIGHT){
    int x2 = get_x2y2(handle, release).x;
    int y2 = handle.GetCanvasSize().h - 1;
    return IntRect(IntPoint(release.x, 0), IntPoint(x2, y2));
  }
  assert(false);
  return IntRect();
}

static IntSize get_rescale_size(const CanvasResizeHandle& handle,
  const IntPoint& release)
{
  HandleDirection dir = handle.GetDirection();
  IntSize rectSize = get_resize_rect(handle, release).GetSize();

  if (dir == HandleDirection::DIAGONAL){
    return rectSize;
  }
  else if (dir == HandleDirection::UP_DOWN){
    return IntSize(handle.GetCanvasSize().w, rectSize.h);
  }
  else if (dir == HandleDirection::LEFT_RIGHT){
    return IntSize(rectSize.w, handle.GetCanvasSize().h);
  }
  assert(false);
  return IntSize(0,0);
}

static bool nearest_neighbour(const ToolModifiers& mod){
  return mod.Primary();
}

static void set_rescale_status(StatusInterface& status,
  const CanvasResizeHandle& handle,
  const IntPoint& release,
  const ToolModifiers& modifiers)
{
  HandleDirection dir = handle.GetDirection();
  if (constrain_resize(modifiers, dir) && nearest_neighbour(modifiers)){
    status.SetMainText("Scale nearest proportional.");
  }
  else if (nearest_neighbour(modifiers)) {
    status.SetMainText(can_constrain(dir) ?
      space_sep("Scale nearest.", secondary_modifier("Constrain")) :
      "Scale nearest");
  }
  else if (constrain_resize(modifiers, dir)){
    status.SetMainText(space_sep("Scale bilinear proportional.",
      primary_modifier("Nearest neighbour")));;
  }
  else {
    status.SetMainText(can_constrain(dir) ?
      space_sep(primary_modifier("Nearest neighbour"),
        secondary_modifier("Proportional")) :
      primary_modifier("Nearest neighbour"));
  }

  IntSize imageSize = handle.GetCanvasSize();
  IntRect r(get_resize_rect(handle, release));
  if (constrain_resize(modifiers, dir)){
    status.SetText(str_percentage(r.w , imageSize.w));
  }
  else {
    if (dir == HandleDirection::UP_DOWN){
      status.SetText(str_percentage(r.h, imageSize.h));
    }
    else if (dir == HandleDirection::LEFT_RIGHT){
      status.SetText(str_percentage(r.w, imageSize.w));
    }
    else {
      status.SetText(comma_sep(str_percentage(r.w, imageSize.w),
        str_percentage(r.h, imageSize.h)));
    }
  }
}

static void set_resize_status(StatusInterface& status,
  const CanvasResizeHandle& handle,
  const IntPoint& pos,
  const ToolModifiers& modifiers)
{
  HandleDirection dir = handle.GetDirection();

  if (constrain_resize(modifiers, dir)){
    status.SetMainText("Proportional resize");
  }
  else {
    status.SetMainText(can_constrain(dir) ?
      secondary_modifier("Proportional")
      : utf8_string());
  }

  const IntRect r(get_resize_rect(handle, pos));
  if (dir == HandleDirection::LEFT_RIGHT){
    status.SetText(space_sep(lbl("x", pos.x), lbl("w", r.w)));
  }
  else if (dir == HandleDirection::UP_DOWN){
    status.SetText(space_sep(lbl("y", pos.y), lbl("h", r.h)));
  }
  else {
    status.SetText(space_sep(lbl("Pos", str(pos)),
      lbl("Size", str(r.GetSize()))));
  }
}

Settings resize_canvas_settings(const Settings& allSettings){
  Settings s;
  s.Set(ts_Bg, allSettings.Get(ts_Bg));
  return s;
}

class ResizeCanvasTool : public StandardTool {
public:
  enum Operation { RESIZE, SCALE };

  ResizeCanvasTool(const CanvasResizeHandle& handle,
    Operation op,
    const Settings& allSettings)
    : StandardTool(ToolId::OTHER, resize_canvas_settings(allSettings)),
      m_handle(handle),
      m_release(handle.GetPos()),
      m_operation(op),
      m_quality(ScaleQuality::BILINEAR)
  {}

  void Draw(FaintDC&, Overlays& overlays, const PosInfo&) override{
    Settings s(default_rectangle_settings());
    HandleDirection dir = m_handle.GetDirection();
    IntPoint opposite(m_handle.Opposite().GetPos());

    if (dir == HandleDirection::DIAGONAL){
      overlays.Rectangle(floated(get_resize_rect(m_handle, m_release)));
    }
    else if (dir == HandleDirection::LEFT_RIGHT){
      if (m_handle.GetPos().x > opposite.x){
        // Offset to the right of the pixel if adjusting from the right
        coord x = floated(m_release.x) + (m_release.x >= opposite.x ?
          1.0 : 0.0);
        overlays.VerticalLine(x);
      }
      else{
        // Offset to the right of the pixel if adjusting from the right
        coord x = floated(m_release.x) + (m_release.x > opposite.x ?
          1.0 : 0.0);
        overlays.VerticalLine(x);
      }
    }
    else if (dir == HandleDirection::UP_DOWN){
      if (m_handle.GetPos().y > opposite.y){
        // Offset to the bottom of the pixel if adjusting from the bottom
        coord y = floated(m_release.y) + (m_release.y >= opposite.y ?
          1.0 : 0.0);
        overlays.HorizontalLine(y);
      }
      else{
        coord y = floated(m_release.y) + (m_release.y > opposite.y ?
          1.0 : 0.0);
        overlays.HorizontalLine(y);
      }
    }
  }

  bool DrawBeforeZoom(Layer) const override{
    return false;
  }

  Command* GetCommand() override{
    return m_command.Take();
  }

  Cursor GetCursor(const PosInfo&) const override{
    return Cursor::CROSSHAIR;
  }

  IntRect GetRefreshRect(const RefreshInfo& info) const override{
    HandleDirection dir = m_handle.GetDirection();
    IntPoint opposite(m_handle.Opposite().GetPos());
    const IntRect& r(info.visibleRect);

    if (dir == HandleDirection::DIAGONAL){
      return IntRect(m_release, opposite);
    }
    else if (dir == HandleDirection::LEFT_RIGHT){
      return IntRect(IntPoint(m_release.x - 1, r.y - 1), IntSize(3, r.h + 1));
    }
    else if (dir == HandleDirection::UP_DOWN){
      return IntRect(IntPoint(r.x, m_release.y - 1), IntSize(r.w, 3));
    }
    else {
      assert(false);
      return IntRect(IntPoint(0, 0), IntSize(1, 1));
    }
  }

  ToolResult MouseDown(const PosInfo&) override{
    return ToolResult::NONE;
  }

  ToolResult MouseUp(const PosInfo& info) override{
    Point pos = info.pos;
    info.status.SetMainText("");
    info.status.SetText("");
    HandleDirection dir(m_handle.GetDirection());
    IntPoint opposite(m_handle.Opposite().GetPos());
    if (constrain_resize(info.modifiers, dir)) {
      pos = constrain_proportional(pos, floated(opposite),
        floated(m_handle.GetPos()));
    }
    m_release = floored(pos);
    CreateCommand(info);
    return ToolResult::CHANGE;
  }

  ToolResult MouseMove(const PosInfo& info) override{
    Point pos = info.pos;
    IntPoint opposite(m_handle.Opposite().GetPos());
    if (constrain_resize(info.modifiers, m_handle.GetDirection())){
      pos = constrain_proportional(pos, floated(opposite),
        floated(m_handle.GetPos()));
    }

    m_quality = nearest_neighbour(info.modifiers) ? ScaleQuality::NEAREST :
      ScaleQuality::BILINEAR;

    m_release = floored(pos);

    if (m_operation == SCALE){
      set_rescale_status(info.status, m_handle, m_release, info.modifiers);
    }
    else {
      set_resize_status(info.status, m_handle, m_release, info.modifiers);
    }
    return ToolResult::DRAW;
  }

  ToolResult Preempt(const PosInfo&) override{
    return ToolResult::CHANGE;
  }

private:
  void CreateCommand(const PosInfo& info){
    if (m_operation == RESIZE){
      IntRect rect(get_resize_rect(m_handle, m_release));
      if (rect != image_rect(info.canvas.GetImage())){
        m_command.Set(get_resize_command(info.canvas.GetBitmap(), rect,
          GetSettings().Get(ts_Bg)));
      }
    }
    else {
      assert(m_operation == SCALE);
      IntSize newSize = get_rescale_size(m_handle, m_release);
      if (newSize != info.canvas.GetSize()){
        m_command.Set(rescale_command(newSize, m_quality));
      }
    }
  }

  CanvasResizeHandle m_handle;
  PendingCommand m_command;
  IntPoint m_release;
  Operation m_operation;
  ScaleQuality m_quality;
};

Tool* resize_canvas_tool(const CanvasResizeHandle& handle,
  const Settings& allSettings)
{
  return new ResizeCanvasTool(handle, ResizeCanvasTool::RESIZE, allSettings);
}

Tool* scale_canvas_tool(const CanvasResizeHandle& handle,
  const Settings& allSettings)
{
  return new ResizeCanvasTool(handle, ResizeCanvasTool::SCALE, allSettings);
}

} // namespace
