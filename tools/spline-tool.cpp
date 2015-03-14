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

#include "geo/geo-func.hh"
#include "geo/int-rect.hh"
#include "geo/measure.hh"
#include "geo/points.hh"
#include "objects/objspline.hh"
#include "rendering/faint-dc.hh"
#include "text/formatting.hh"
#include "tools/standard-tool.hh"
#include "util/command-util.hh"
#include "util/default-settings.hh"
#include "util/pos-info.hh"
#include "util/undo-redo.hh"

namespace faint{

static Settings spline_tool_settings(const Settings& allSettings){
  Settings s(default_spline_settings());
  s.Update(allSettings);
  return s;
}


class SplineTool : public StandardTool, public HistoryContext{
public:
  SplineTool(const Settings& allSettings)
    : StandardTool(ToolId::SPLINE, spline_tool_settings(allSettings)),
      m_otherButton(MouseButton::RIGHT)
  {}

  bool AllowsGlobalRedo() const override{
    return !m_active;
  }

  bool CanRedo() const override{
    return m_active && m_states.CanRedo();
  }

  bool CanUndo() const override{
    return m_active && m_states.CanUndo();
  }

  void Draw(FaintDC& dc, Overlays&, const PosInfo& posInfo) override{
    if (m_active){
      m_points.Append(posInfo.pos);
      dc.Spline(m_points.GetPointsDumb(), GetSettings());
      m_points.PopBack();
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

  IntRect GetRefreshRect(const RefreshInfo& info) const override{
    return floored(inflated(bounding_rect(m_p1, m_p2, info.mousePos),
      GetSettings().Get(ts_LineWidth)));
  }

  utf8_string GetUndoName() const override{
    return "Add Point";
  }

  Optional<const faint::HistoryContext&> HistoryContext() const override{
    return Optional<const faint::HistoryContext&>(*this);
  }

  ToolResult MouseDown(const PosInfo& info) override{
    if (m_active && m_otherButton == info.modifiers.MouseButton()){
      m_points.Append(info.pos);
      return Commit(info.layerType);
    }
    else if (!m_active){
      m_active = true;
      m_p1 = info.pos;
      m_p2 = info.pos;
      m_points.Append(info.pos);

      SetSwapColors(info.modifiers.RightMouse());
      m_otherButton = the_other_one(info.modifiers.MouseButton());
    }

    m_points.Append(info.pos);
    m_states.Did(info.pos);
    m_p1 = min_coords(m_p1, info.pos);
    m_p2 = max_coords(m_p2, info.pos);
    return ToolResult::NONE;
  }

  ToolResult MouseUp(const PosInfo&) override{
    return ToolResult::NONE;
  }

  ToolResult MouseMove(const PosInfo& info) override{
    info.status.SetText(str(info.pos), 0);
    if (m_active){
      m_points.AdjustBack(info.pos);
      StrBtn btn(m_otherButton);
      info.status.SetMainText(space_sep(btn.Other(true), "click to add points,",
          btn.This(false), "click to stop."));
      return ToolResult::DRAW;
    }
    else {
      info.status.SetMainText("Click to start drawing a spline");
    }
    return ToolResult::NONE;
  }


  ToolResult Preempt(const PosInfo& info) override{
    if (m_active){
      return Commit(info.layerType);
    }
    return ToolResult::NONE;
  }

  void Redo() override{
    m_points.Append(m_states.Redo());
  }

  void Undo() override{
    m_states.Undo();
    m_points.PopBack();
    if (!m_states.CanUndo()){
      m_active = false;
      m_states.Clear();
      m_points.Clear();
    }
  }

private:
  ToolResult Commit(Layer layer){
    m_command.Set(add_or_draw(create_spline_object(m_points,
      GetSettings()), layer));
    m_states.Clear();
    m_points.Clear();
    m_active = false;
    return ToolResult::COMMIT;
  }

  Point m_p1;
  Point m_p2;
  bool m_active = false;
  Points m_points;
  MouseButton m_otherButton;
  PendingCommand m_command;
  UndoRedo<Point> m_states;
};

Tool* spline_tool(const Settings& allSettings){
  return new SplineTool(allSettings);
}

} // namespace
