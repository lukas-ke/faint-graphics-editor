// -*- coding: us-ascii-unix -*-
// Copyright 2013 Lukas Kemmer
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
#include "commands/flip-rotate-cmd.hh"
#include "editors/line-editor.hh"
#include "geo/geo-func.hh"
#include "geo/int-rect.hh"
#include "geo/line.hh"
#include "geo/measure.hh"
#include "geo/rect.hh"
#include "geo/scale.hh"
#include "rendering/overlay.hh"
#include "text/formatting.hh"
#include "tools/standard-tool.hh"
#include "util/command-util.hh"
#include "util/object-util.hh"
#include "util/pos-info.hh"

namespace faint{

static bool target_selected_objects(const PosInfo& info){
  return info.layerType == Layer::OBJECT &&
    !(info.canvas.GetObjectSelection().empty());
}

static bool define_horizon(const PosInfo& info){
  return info.modifiers.RightMouse();
}

static bool scale(const PosInfo& info, const Optional<LineSegment>& horizon){
  return horizon.IsSet() &&
    info.modifiers.Primary()
    && !define_horizon(info);
}

static bool rather_short(const LineSegment& line){
  return length(line) < 5.0;
}

// Returns the shortest rotation angle that would align the line with
// the horizon (0 degrees)
static Angle get_rotation_angle(const LineSegment& line){
  Angle angle = angle360(line);
  if (angle <= Angle::Deg(90)){
    return angle;
  }
  else if (angle <= Angle::Deg(270)){
    return angle - pi;
  }
  return angle - 2 * pi;
}

static Settings level_tool_settings(const Settings& allSettings){
  // Use the background color to fill the outside when rotating.
  Settings s;
  s.Set(ts_Fg, Paint(Color(0, 0, 0)));
  s.Set(ts_Bg, Paint(Color(0, 0, 0)));
  s.Set(ts_SwapColors, false);
  s.Update(allSettings);
  return s;
}

static void level_line_status(const PosInfo& info,
  const Angle& a,
  bool horizonDefined)
{
  if (horizonDefined && !info.modifiers.Primary()){
    info.status.SetMainText(primary_modifier("Scale"));
  }

  info.status.SetText(str_degrees_int_symbol(truncated(a.Deg())), 0);
}

class LevelTool : public StandardTool{
  // Tool for rotating the image so that the specified line becomes
  // horizontal.
public:
  LevelTool(const Settings& allSettings) :
    StandardTool(ToolId::LEVEL, level_tool_settings(allSettings)),
    m_active(false),
    m_editor(AllowConstrain(false), // Would interfere with scale option
      AllowSnap(true))
  {}

  void Draw(FaintDC&, Overlays& overlays, const PosInfo&) override{
    if (m_active){
      overlays.Line(m_editor.GetLine());
    }
  }

  bool DrawBeforeZoom(Layer) const override{
    return true;
  }

  Command* GetCommand() override{
    return m_command.Take();
  }

  Cursor GetCursor(const PosInfo& info) const override{
    if (define_horizon(info)){
      return Cursor::LEVEL_DEFINE;
    }
    else if (scale(info, m_horizon)){
      return Cursor::LEVEL_SCALE;
    }
    else{
      return Cursor::LEVEL;
    }
  }

  IntRect GetRefreshRect(const RefreshInfo&) const override{
    return m_editor.GetRefreshRect();
  }

  ToolResult MouseDown(const PosInfo& info) override{
    m_editor.MouseDown(info);
    level_line_status(info, GetAngle(), m_horizon.IsSet());
    m_active = true;
    return ToolResult::DRAW;
  }

  ToolResult MouseUp(const PosInfo& info) override{
    if (!m_active){
      return ToolResult::NONE;
    }

    m_editor.MouseUp(info);
    level_line_status(info, GetAngle(), m_horizon.IsSet()); // Fixme
    m_active = false;

    const auto line = m_editor.GetLine();

    if (define_horizon(info)){
      if (rather_short(line)){
        m_horizon.Clear();
      }
      else{
        m_horizon.Set(line);
      }
      return ToolResult::CANCEL;
    }

    if (rather_short(line)){
      return ToolResult::CANCEL;
    }

    if (target_selected_objects(info)){
      objects_t objects = info.canvas.GetObjectSelection();
      Point origin = bounding_rect(objects).Center();
      if (scale(info, m_horizon)){
        m_command.Set(get_scale_rotate_command(objects,
            Scale(length(m_horizon.Get()) / length(line)),
            GetAngle(),
            origin));
      }
      else{
        m_command.Set(get_rotate_command(objects, GetAngle(), origin));
      }
      return ToolResult::COMMIT;
    }

    if (scale(info, m_horizon)){
      const auto scaling = Scale(length(m_horizon.Get()) / length(line));
      const auto angle = GetAngle();
      auto newSize = rotate_scale_bilinear_size(info.canvas.GetSize(),
        angle, scaling);
      if (!reasonable_bitmap_size(newSize)){
        // Fixme: Signal to user somehow.
        return ToolResult::CANCEL;
      }

      m_command.Set(rotate_scale_image_bilinear_command(angle,
        scaling,
        GetSettings().Get(ts_Bg)));
    }
    else{
      // Fixme: Check size here too, rotation can also grow the image
      m_command.Set(rotate_image_command(GetAngle(),
          GetSettings().Get(ts_Bg)));
    }
    return ToolResult::COMMIT;
  }

  ToolResult MouseMove(const PosInfo& info) override{
    if (!m_active){
      if (target_selected_objects(info)){
        info.status.SetMainText(space_sep("Draw line to rotate to",
          str_degrees_symbol(GetHorizonAngle()),
          "(selected objects)", "Right click to define horizon."));
      }
      else{
        info.status.SetMainText(space_sep("Draw a line to rotate to",
          str_degrees_symbol(GetHorizonAngle()) + ".",
          "Right click to define horizon."));
      }
      info.status.SetText(str(info.pos), 0);
      return ToolResult::NONE;
    }
    else{
      if (define_horizon(info)){
        if (rather_short(m_editor.GetLine())){
          info.status.SetMainText("Release to reset horizon to " +
            str_degrees_int_symbol(0));
        }
        else{
          info.status.SetMainText("Release to define horizon");
        }
      }
      else{
        info.status.SetMainText("");
      }

      m_editor.MouseMove(info);
      level_line_status(info, GetAngle(), m_horizon.IsSet());
    }
    return ToolResult::DRAW;
  }

  ToolResult Preempt(const PosInfo&) override{
    return ToolResult::NONE;
  }

private:
  Angle GetAngle() const{
    return get_rotation_angle(m_editor.GetLine()) - GetHorizonAngle();
  }

  Angle GetHorizonAngle() const{
    return m_horizon.IsSet() ?
      get_rotation_angle(m_horizon.Get()) :
      Angle::Zero();
  }

  bool m_active;
  PendingCommand m_command;
  LineEditor m_editor;
  Optional<LineSegment> m_horizon;
};

Tool* level_tool(const Settings& allSettings){
  return new LevelTool(allSettings);
}

} // namespace
