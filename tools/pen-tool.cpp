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

#include <vector>
#include "bitmap/color.hh"
#include "commands/command.hh"
#include "geo/adjust.hh"
#include "geo/geo-func.hh"
#include "geo/int-rect.hh"
#include "geo/int-size.hh"
#include "geo/measure.hh"
#include "geo/int-point.hh"
#include "geo/point.hh"
#include "rendering/faint-dc.hh"
#include "text/formatting.hh"
#include "tools/standard-tool.hh"
#include "util/container-util.hh"
#include "util/setting-util.hh"
#include "util/pos-info.hh"

namespace faint{

class PenCommand : public Command {
public:
  PenCommand(const std::vector<IntPoint>& points, const Settings& settings)
    : Command(CommandType::RASTER),
      m_settings(settings),
      m_points(points)
  {
    finalize_swap_colors_erase_bg(m_settings);
  }

  void Do(CommandContext& context){
    context.GetDC().PenStroke(m_points, m_settings);
  }

  utf8_string Name() const{
    return "Pen Stroke";
  }
private:
  Settings m_settings;
  std::vector<IntPoint> m_points;
};

static bool pen_constrain_held(const ToolModifiers& modifiers){
  return modifiers.Secondary();
}

static Settings pen_settings(const Settings& allSettings){
  Settings s;
  s.Set(ts_Fg, Paint(Color(0,0,0)));
  s.Set(ts_Bg, Paint(Color(0,0,0)));
  s.Set(ts_SwapColors, false);
  s.Update(allSettings);
  return s;
}

class PenTool : public StandardTool {
public:
  PenTool(const Settings& allSettings)
    : StandardTool(ToolId::PEN, pen_settings(allSettings)),
      m_active(false),
      m_constrainDir(ConstrainDir::NONE)
  {}

  void Draw(FaintDC& dc, Overlays&, const PosInfo&) override{
    if (m_active){
      dc.PenStroke(m_points, GetSettings());
    }
  }

  bool DrawBeforeZoom(Layer) const override{
    return true;
  }

  Command* GetCommand() override{
    return m_command.Take();
  }

  Cursor GetCursor(const PosInfo&) const override{
    return Cursor::PEN;
  }

  IntRect GetRefreshRect(const RefreshInfo&) const override{
    if (m_points.empty()){
      return {};
    }
    else if (m_points.size() == 1){
      return IntRect(m_points[0], IntSize(1,1));
    }
    else{
      IntPoint p0 = m_points[ m_points.size() -1 ];
      IntPoint p1 = m_points[ m_points.size() -2 ];
      return IntRect(p0, p1);
    }
  }

  ToolResult MouseDown(const PosInfo& info) override{
    m_active = true;
    m_origin = floored(info.pos);
    m_constrainDir = ConstrainDir::NONE;
    SetSwapColors(info.modifiers.RightMouse());
    m_points.clear();
    m_points.push_back(m_origin);
    return ToolResult::DRAW;
  }

  ToolResult MouseUp(const PosInfo&) override{
    if (!m_active){
      return ToolResult::NONE;
    }
    return Commit();
  }

  ToolResult MouseMove(const PosInfo& info) override{
    Point pos = info.pos;
    info.status.SetText(str(pos), 0);

    if (!m_active){
      m_constrainDir = ConstrainDir::NONE;
      return ToolResult::NONE;
    }

    if (pen_constrain_held(info.modifiers)) {
      if (m_constrainDir == ConstrainDir::NONE){
        if (distance(floored(pos), m_origin) > 5){
          // Lock the constrain direction after a certain distance
          // to avoid weird switches
          m_constrainDir = constrain_pos(pos, floated(m_origin));
        }
        else {
          constrain_pos(pos, floated(m_origin));
        }
      }
      else {
        constrain_pos(pos, floated(m_origin), m_constrainDir);
      }
    }
    else {
      m_origin = floored(pos);
      m_constrainDir = ConstrainDir::NONE;
    }

    IntPoint intPos(floored(pos));
    if (get_item(m_points, -1) != intPos){
      m_points.push_back(intPos);
    }
    return ToolResult::DRAW;
  }

  ToolResult Preempt(const PosInfo&) override{
    if (!m_active){
      return ToolResult::NONE;
    }
    return Commit();
  }

private:
  ToolResult Commit(){
    m_command.Set(new PenCommand(m_points, GetSettings()));
    m_active = false;
    SetSwapColors(false);
    m_points.clear();
    m_constrainDir = ConstrainDir::NONE;
    return ToolResult::COMMIT;
  }

  bool m_active;
  PendingCommand m_command;
  ConstrainDir m_constrainDir;
  IntPoint m_origin;
  std::vector<IntPoint> m_points;
};

Tool* pen_tool(const Settings& allSettings){
  return new PenTool(allSettings);
}

} // namespace
