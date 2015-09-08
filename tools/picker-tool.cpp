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
#include "bitmap/pattern.hh"
#include "geo/geo-func.hh"
#include "geo/int-rect.hh"
#include "text/formatting.hh"
#include "tools/standard-tool.hh"
#include "tools/tool-actions.hh"
#include "util/pos-info.hh"
#include "util/tool-util.hh"

namespace faint{

static bool should_pick_to_pattern(const PosInfo& info){
  return info.modifiers.Primary();
}

static bool should_anchor_topleft(const PosInfo& info){
  return info.modifiers.Secondary();
}

static utf8_string picker_description(const PosInfo& info){
  return
    should_pick_to_pattern(info) ?
      should_anchor_topleft(info) ?
        "Pick as pattern, anchor top left" :
        "Pick as pattern (Shift=anchor top left)" :
    "Ctrl=Pick as pattern";
}

static Paint pattern_get_hovered_paint(const PosInside& info){
  // Pick either the hovered object fill, the color at the pixel in
  // the background or floating raster selection.
  // Do not pick invisible object insides.
  return get_hovered_paint(info,
    include_hidden_fill(false),
    include_floating_selection(true));
}

static Paint pick_to_pattern(const PosInside& info, const Bitmap& bg){
  // Pick the background bitmap as a pattern, anchored around either
  // the click-point or the top-left corner
  bool anchorTopLeft = should_anchor_topleft(info);
  IntPoint anchor = anchorTopLeft ?
    IntPoint(0,0) : // Image top left
    floored(info->pos);
  Pattern pattern(bg, anchor, object_aligned_t(!anchorTopLeft));
  return Paint(pattern);
}

static ToolResult pick(const PosInside& info, ToolActions& actions){
  const auto& bg = info->canvas.GetBitmap();

  Paint paint = (should_pick_to_pattern(info) && bg.IsSet()) ?
    pick_to_pattern(info, bg.Get()) :
    pattern_get_hovered_paint(info);

  actions.Set(fg_or_bg(info), paint);
  return ToolResult::NONE;
}

class PickerTool : public StandardTool {
public:
  PickerTool(ToolActions& actions)
    : StandardTool(ToolId::PICKER, Settings()),
      m_actions(actions)
  {}

  void Draw(FaintDC&, Overlays&, const PosInfo&) override{
  }

  bool DrawBeforeZoom(Layer) const override{
    return false;
  }

  Command* GetCommand() override{
    assert(false);
    return nullptr;
  }

  Cursor GetCursor(const PosInfo& info) const override{
    if (should_pick_to_pattern(info)){
      return should_anchor_topleft(info) ?
        Cursor::PICKER_PATTERN_TOPLEFT : Cursor::PICKER_PATTERN;
    }
    return Cursor::PICKER;
  }

  IntRect GetRefreshRect(const RefreshInfo&) const override{
    return {};
  }

  ToolResult MouseDown(const PosInfo& info) override{
    return inside_canvas(info).Visit(
      [&](const PosInside& insidePos){
        return pick(insidePos, m_actions);
      },
      otherwise(ToolResult::NONE));
  }

  ToolResult MouseUp(const PosInfo&) override{
    return ToolResult::NONE;
  }

  ToolResult MouseMove(const PosInfo& info) override{
    info.status.SetMainText(picker_description(info));
    inside_canvas(info).Visit(
      [](const PosInside& info){
        // Inside canvas
        if (should_pick_to_pattern(info)){
          info->status.SetText(should_anchor_topleft(info) ?
            "Anchor: Top Left (0,0)" :
            space_sep("Anchor:", bracketed(str_floor((info->pos)))));
        }
        else{
          Paint paint(pattern_get_hovered_paint(info));
          info->status.SetText(space_sep(str(paint),
            bracketed(str_floor(info->pos))));
        }
      },
      [&info](){
        // Outside
        info.status.SetText(bracketed(str_floor(info.pos)));
      });
    return ToolResult::NONE;
  }

  ToolResult Preempt(const PosInfo&) override{
    return ToolResult::NONE;
  }

  ToolActions& m_actions;
};

Tool* picker_tool(ToolActions& actions){
  return new PickerTool(actions);
}

} // namespace
