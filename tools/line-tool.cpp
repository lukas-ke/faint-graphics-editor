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
#include "app/canvas.hh"
#include "geo/adjust.hh"
#include "geo/geo-func.hh"
#include "geo/int-rect.hh"
#include "geo/line.hh"
#include "geo/points.hh"
#include "objects/objline.hh"
#include "rendering/faint-dc.hh"
#include "text/formatting.hh"
#include "tools/standard-tool.hh"
#include "util/command-util.hh"
#include "util/container-util.hh"
#include "util/default-settings.hh"
#include "util/grid.hh"
#include "util/object-util.hh"
#include "util/pos-info.hh"
#include "util/setting-util.hh"
#include "util/undo-redo.hh"
#include "util-wx/key-codes.hh"

namespace faint{

static Settings clean_line_object_settings(const Settings& oldSettings){
  Settings s(remove_background_color(oldSettings));
  // Polyline is only a tool setting, for controlling the point adding.
  s.Erase(ts_PolyLine);
  return s;
}

static Point line_constrain(const Point& p0, const Point& p1, bool subPixel){
  return subPixel ?
    adjust_to_45(p0, p1) :
    adjust_to_45(floated(floored(p0)), floated(floored(p1)));
}

static void line_status_bar(StatusInterface& status,
  const LineSegment& line,
  const ToolModifiers& modifiers,
  bool subPixel)
{
  if (modifiers.Neither()) {
    status.SetMainText(space_sep(primary_modifier("Snap"),
      secondary_modifier("Constrain")));
  }
  else {
    status.SetMainText("");
  }

  status.SetText((subPixel ?
      str_line_status_subpixel(line) :
      str_line_status(floored(line))),
    0);
}

static bool coincident(const std::vector<Point>& points){
  return points.size() == 2 && points[0] == points[1];
}

static coord refresh_width(const Settings& s){
  return s.Get(ts_LineArrowhead) == LineArrowhead::FRONT ?
    s.Get(ts_LineWidth) * 10.0 :
    s.Get(ts_LineWidth);
}

static Settings line_settings(const Settings& allSettings){
  Settings s(default_line_settings());
  s.Set(ts_PolyLine,false);
  s.Set(ts_Bg,Paint(Color(0,0,0)));
  s.Update(allSettings);
  return s;
}

class LineTool : public StandardTool, public HistoryContext{
public:
  LineTool(const Settings& allSettings)
    : StandardTool(ToolId::LINE, line_settings(allSettings)),
      m_otherButton(MouseButton::RIGHT)
  {}

  bool AllowsGlobalRedo() const override{
    return !CanRedo();
  }

  bool CanRedo() const override{
    return m_states.CanRedo();
  }

  bool CanUndo() const override{
    return m_active && m_states.CanUndo();
  }

  ToolResult Char(const KeyInfo& info) override{
    if (!m_active || GetSettings().Not(ts_PolyLine)){
      return ToolResult::NONE;
    }
    if (info.key.Is(key::esc)){
      // Commit a drawn polyline on escape (though the preferred
      // method is right click)
      return Commit(info.layerType);
    }
    return ToolResult::NONE;
  }

  void Draw(FaintDC& dc, Overlays&, const PosInfo&) override{
    if (m_active){
      dc.PolyLine(tri_from_points(m_points), m_points,
        clean_line_object_settings(GetSettings()));
    }
  }

  bool DrawBeforeZoom(Layer layer) const override{
    return layer == Layer::RASTER;
  }

  Command* GetCommand() override{
    return m_command.Take();
  }

  Cursor GetCursor(const PosInfo&) const override{
    return Cursor::CROSSHAIR;
  }

  utf8_string GetRedoName() const override{
    return "Add Point";
  }

  IntRect GetRefreshRect(const RefreshInfo&) const override{
    if (m_points.size() < 2){
      return {};
    }
    else{
      return floored(inflated(Rect(last_two(m_points)),
        refresh_width(GetSettings())));
    }
  }

  utf8_string GetUndoName() const override{
    return "Add Point";
  }

  Optional<const faint::HistoryContext&> HistoryContext() const override{
    return Optional<const faint::HistoryContext&>(*this);
  }

  ToolResult MouseDown(const PosInfo& info) override{
    const auto& modifiers = info.modifiers;
    const bool first = !m_active;
    if (first){
      Reset(info.modifiers);
    }
    SetAntiAlias(info);
    const Point pos(GetPos(info, first));
    const Settings& s(GetSettings());
    if (s.Get(ts_PolyLine) && modifiers.Feature(m_otherButton)){
      // Complete the a poly-line if the opposite mouse-button of the initially
      // used is clicked.
      return Commit(info.layerType);
    }
    else{
      m_points.push_back(pos);
      m_states.Did(pos);
      if (first){
        m_points.push_back(pos);
      }
      return UpdateStatusBar(info);
    }
  }

  ToolResult MouseUp(const PosInfo& info) override{
    if (!m_active || GetSettings().Get(ts_PolyLine)){
      return ToolResult::NONE;
    }
    else{
      m_command.Set(CreateCommand(info.layerType));
      return m_command.Valid() ? ToolResult::COMMIT : ToolResult::NONE;
    }
  }

  ToolResult MouseMove(const PosInfo& info) override{
    if (!m_active){
      info.status.SetMainText("");
      info.status.SetText(str(info.pos), 0);
      return ToolResult::NONE;
    }
    else{
      SetAntiAlias(info);
      m_points.back() = GetPos(info, false);
      return UpdateStatusBar(info);
    }
  }

  ToolResult Preempt(const PosInfo& info) override{
    if (m_active){
      m_command.Set(CreateCommand(info.layerType));
      return m_command.Valid() ? ToolResult::COMMIT : ToolResult::CANCEL;
    }
    else{
      return ToolResult::NONE;
    }
  }

  void Redo() override{
    m_points.pop_back();
    m_points.push_back(m_states.Redo());
    m_points.push_back(m_points.back()); // Cursor position
    m_active = m_points.size() >= 2;
  }

  void Undo() override{
    assert(m_states.CanUndo());
    m_points.pop_back();
    m_states.Undo();
    m_active = m_points.size() >= 2;
  }

private:
  ToolResult Commit(Layer layerType){
    m_command.Set(CreateCommand(layerType));
    return m_command.Valid() ? ToolResult::COMMIT : ToolResult::NONE;
  }

  Command* CreateCommand(Layer layer){
    assert(m_points.size() >= 2);
    std::vector<Point> points(std::move(m_points));
    m_states.Clear();
    m_active = false;

    if (layer == Layer::OBJECT && coincident(points)){
      // Prevent 0-length object lines, as they're invisible (Such
      // raster lines are allowed - it's the same as drawing a pixel)
      return nullptr;
    }

    return add_or_draw(create_line_object(Points(to_line_path(points)),
      clean_line_object_settings(GetSettings())), layer);
  }

  Point GetPos(const PosInfo& info, bool first){
    if (first){
      return info.modifiers.Primary() ?
        snap(info.pos, info.canvas.GetObjects(), info.canvas.GetGrid()) :
        info.pos;
    }
    else if (info.modifiers.Primary()){
      return snap(info.pos, info.canvas.GetObjects(),
        info.canvas.GetGrid());
    }
    else if (info.modifiers.Secondary() && m_points.size() > 1){
      return line_constrain(get_item(m_points, -2), info.pos, sub_pixel(info));
    }
    else{
      return info.pos;
    }
  }

  void Reset(const ToolModifiers& modifiers){
    m_points.clear();
    m_states.Clear();
    m_active = true;
    m_otherButton = the_other_one(modifiers.MouseButton());
    SetSwapColors(modifiers.RightMouse());
  }

  ToolResult UpdateStatusBar(const PosInfo& info){
    assert(m_points.size() >= 2);
    line_status_bar(info.status,
      last_two(m_points), info.modifiers, GetAntiAlias());

    return ToolResult::DRAW;
  }

  std::vector<Point> m_points;
  bool m_active = false;
  MouseButton m_otherButton = MouseButton::RIGHT;
  PendingCommand m_command;
  UndoRedo<Point> m_states;
};

Tool* line_tool(const Settings& allSettings){
  return new LineTool(allSettings);
}

} // namespace
