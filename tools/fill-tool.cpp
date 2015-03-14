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

#include "app/canvas.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "commands/change-setting-cmd.hh"
#include "geo/geo-func.hh"
#include "geo/int-rect.hh"
#include "text/formatting.hh"
#include "tools/standard-tool.hh"
#include "util/color-span.hh"
#include "util/command-util.hh"
#include "util/object-util.hh"
#include "util/pos-info.hh"
#include "util/setting-util.hh"
#include "util/tool-util.hh"

namespace faint{

static bool no_fillable_object_hit(const PosInfo& info){
  return !object_color_region_hit(info) || is_raster_object(info.object);
}

static utf8_string hit_description(const PosInfo& info){
  if (!info.object->GetSettings().Has(ts_Bg)){
    return "color";
  }
  else {
    return info.hitStatus == Hit::BOUNDARY ? "edge color" : "fill color";
  }
}

static Color fill_tool_get_replace_color(const PosInfo& info){
  return info.canvas.GetBackground().Visit(
    [&](const Bitmap& bmp){
      return get_color(bmp, floored(info.pos));
    },
    [](const ColorSpan& span){
      return span.color;
    });
}

static bool fill_boundary_flag(const PosInfo& info, const Settings& s){
  const auto& modifiers = info.modifiers;
  const auto boundarySetting = the_other_one(fg_or_bg(info));
  return modifiers.OnlySecondary() && s.Get(boundarySetting).IsColor();
}

static bool fill_replace_flag(const ToolModifiers& modifiers){
  return modifiers.OnlyPrimary();
}

static bool fill_replace_other_flag(const ToolModifiers& modifiers){
  return modifiers.Both();
}

static Settings fill_settings(const Settings& allSettings){
  Settings s;
  s.Set(ts_Fg, Paint(Color(0,0,0)));
  s.Set(ts_Bg, Paint(Color(0,0,0)));
  s.Update(allSettings);
  return s;
}

class FillTool : public StandardTool {
public:
  FillTool(const Settings& allSettings)
    : StandardTool(ToolId::FLOOD_FILL, fill_settings(allSettings))
  {}

  void Draw(FaintDC&, Overlays&, const PosInfo&) override{
  }

  bool DrawBeforeZoom(Layer) const override{
    return false;
  }

  Command* GetCommand() override{
    return m_command.Take();
  }

  Cursor GetCursor(const PosInfo& info) const override{
    if (fill_replace_flag(info.modifiers)){
      return Cursor::BUCKET_REPLACE;
    }
    else if (fill_replace_other_flag(info.modifiers)){
      return Cursor::BUCKET_REPLACE_OTHER;
    }
    else if (fill_boundary_flag(info, GetSettings())){
      return Cursor::BUCKET_BOUNDARY;
    }
    return Cursor::BUCKET;
  }

  IntRect GetRefreshRect(const RefreshInfo&) const override{
    return {};
  }

  ToolResult MouseDown(const PosInfo& info) override{
    if (outside_canvas(info)){
      return ToolResult::NONE;
    }

    ColorSetting fillSetting(fg_or_bg(info));
    const Settings& s = GetSettings();
    const Paint fill = s.Get(fillSetting);
    if (is_raster(info.layerType) || no_fillable_object_hit(info)){
      // Raster-layer flood fill
      if (fill_replace_flag(info.modifiers)){
        Color clickedColor(fill_tool_get_replace_color(info));
        if (fill == clickedColor){
          // No reason to replace with same color.
          return ToolResult::NONE;
        }
        else {
          m_command.Set(target_full_image(
            get_replace_color_command(Old(clickedColor), fill)));
        }
      }
      else if (fill_replace_other_flag(info.modifiers)){
        Color clickedColor(fill_tool_get_replace_color(info));
        if (fill == clickedColor){
          // This would clear the image, and would be a strange way to clear
          // an image, so do nothing.
          return ToolResult::NONE;
        }
        else {
          m_command.Set(target_full_image(
            get_erase_but_color_command(clickedColor, fill)));
        }
      }
      else if (fill_boundary_flag(info, s)){
        Paint boundary = s.Get(the_other_one(fillSetting));
        assert(boundary.IsColor());
        m_command.Set(target_full_image(
          get_boundary_fill_command(floored(info.pos), fill,
            boundary.GetColor())));
      }
      else {
        m_command.Set(target_full_image(
          get_flood_fill_command(floored(info.pos), fill)));
      }
    }
    else if (is_text_object(info.object)){
      // Text objects don't quite support a background color, and
      // especially not a fill style, so handle them separately
      m_command.Set(get_fill_boundary_command(info.object, fill));
    }
    else if (info.hitStatus == Hit::BOUNDARY){
      m_command.Set(get_fill_boundary_command(info.object, fill));
    }
    else if (info.hitStatus == Hit::INSIDE){
      // Note: DWIM would possible here if the object's fillstyle is BORDER
      // (transparent inside) but that could mean any object below
      // should be filled, or the raster background, so it's tricky...
      m_command.Set(get_fill_inside_command(info.object, fill));
    }
    else {
      assert(false);
    }
    return ToolResult::COMMIT;
  }

  ToolResult MouseUp(const PosInfo&) override{
    return ToolResult::NONE;
  }

  ToolResult MouseMove(const PosInfo& info) override{
    inside_canvas(info).Visit(
      [this](const PosInside& info){
        const Settings& s(GetSettings());
        if (is_object(info->layerType) && object_color_region_hit(info)){
          info->status.SetMainText(space_sep("Click to set the",
            info->object->GetType(), hit_description(info)));
        }
        else{
          if (fill_replace_flag(info->modifiers)){
            info->status.SetMainText(space_sep("Click to replace color",
                bracketed(space_sep(both_modifiers(), "to replace other"))));
          }
          else if (fill_replace_other_flag(info->modifiers)){
            info->status.SetMainText("Click to replace all other colors");
          }
          else if (fill_boundary_flag(info, s)){
            info->status.SetMainText("Click for boundary fill");
          }
          else{
            if (!s.Get(the_other_one(fg_or_bg(info))).IsColor()){
              info->status.SetMainText(primary_modifier("Replace"));
            }
            else{
              info->status.SetMainText(space_sep(primary_modifier("Replace"),
                secondary_modifier("Boundary")));
            }
          }
        }
        info->status.SetText(space_sep(str(get_hovered_paint(info,
          include_hidden_fill(true),
          include_floating_selection(false))),
          bracketed(str(info->pos))));

      },
      [&info](){
        // Outside
        info.status.SetMainText("");
        info.status.SetText(bracketed(str(info.pos)));
      });
    return ToolResult::NONE;
  }

  ToolResult Preempt(const PosInfo&) override{
    return ToolResult::NONE;
  }

private:
  PendingCommand m_command;
};

Tool* fill_tool(const Settings& s){
  return new FillTool(s);
}

} // namespace
