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
#include "commands/command-bunch.hh"
#include "commands/command.hh"
#include "commands/frame-cmd.hh"
#include "geo/geo-func.hh"
#include "rendering/overlay.hh"
#include "text/formatting.hh"
#include "tools/standard-tool.hh"
#include "util/image.hh"
#include "util/index-iter.hh"
#include "util/make-vector.hh"
#include "util/pos-info.hh"

namespace faint{

static IntPoint get_hot_spot(const Canvas& canvas){
  auto frameIndex = canvas.GetSelectedFrame();
  return canvas.GetFrame(frameIndex).GetHotSpot();
}

class HotSpotTool : public StandardTool{
public:
  HotSpotTool()
    : StandardTool(ToolId::HOT_SPOT, Settings())
  {}

  bool DrawBeforeZoom(Layer) const override{
    return false;
  }

  void Draw(FaintDC&, Overlays& overlays, const PosInfo& info) override{
    m_hotSpot = get_hot_spot(info.canvas);
    overlays.Pixel(m_hotSpot);
  }

  Cursor GetCursor(const PosInfo&) const override{
    return Cursor::HOT_SPOT;
  }

  IntRect GetRefreshRect(const RefreshInfo&) const override{
    return IntRect(m_hotSpot - IntPoint(10,10), IntSize(20,20));
  }

  ToolResult MouseDown(const PosInfo& info) override{
    Canvas& canvas = info.canvas;
    HotSpot hotSpot(floored(info.pos));

    if (info.modifiers.LeftMouse()){
      // Command for setting the hot spot to the current frame.

      auto frameIndex = canvas.GetSelectedFrame();
      const Image& frame = canvas.GetFrame(frameIndex);
      m_command.Set(set_frame_hotspot_command(frameIndex, New(hotSpot),
        Old(frame.GetHotSpot())));
      m_hotSpot = hotSpot;
      return ToolResult::COMMIT;
    }
    else if (info.modifiers.RightMouse()){
      // Command for setting the hot-spot to all frames.

      auto commands = make_vector(up_to(canvas.GetNumFrames()),
        [&](const auto& i){
          const Image& frame = canvas.GetFrame(i);
          return set_frame_hotspot_command(i,
            New(hotSpot),
            Old(frame.GetHotSpot()));});

      m_hotSpot = hotSpot;
      m_command.Set(perhaps_bunch(CommandType::FRAME,
          bunch_name("Set frame hot spots"), commands));
      return ToolResult::COMMIT;
    }
    else{
      return ToolResult::NONE;
    }
  }

  ToolResult MouseMove(const PosInfo& info) override{
    info.status.SetMainText("Left click to set hot-spot position "
      "(Right=All frames)");
    info.status.SetText(str(info.pos),0);

    // Update the old hot-spot from the image for showing as an
    // overlay
    m_hotSpot = get_hot_spot(info.canvas);

    return ToolResult::DRAW;
  }

  ToolResult MouseUp(const PosInfo&) override{
    return ToolResult::NONE;
  }

  ToolResult Preempt(const PosInfo&) override{
    return ToolResult::NONE;
  }

  Command* GetCommand() override{
    return m_command.Take();
  }

private:
  IntPoint m_hotSpot;
  PendingCommand m_command;
};

Tool* hot_spot_tool(){
  return new HotSpotTool();
}

} // namespace
