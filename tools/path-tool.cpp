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

#include <memory>
#include <vector>
#include "geo/geo-func.hh"
#include "geo/int-rect.hh"
#include "geo/points.hh"
#include "objects/objpath.hh"
#include "rendering/faint-dc.hh"
#include "text/formatting.hh"
#include "tools/tool.hh"
#include "util/command-util.hh"
#include "util/default-settings.hh"
#include "util/optional.hh"
#include "util/pos-info.hh"
#include "util-wx/key-codes.hh"

namespace faint{

enum class PtResult{
  // Results for actions in a PointBuilder used in the PathTool.
  DRAW, // Tool should draw the current path
  NONE, // Nothing required from tool
  PATH_DONE, // Entire path is completed
  POINT_DONE // Current builder is completed
};

class PointBuilder{
public:
  virtual ~PointBuilder() = default;
  virtual PtResult Motion(const PosInfo&, Points&) = 0;
  virtual PtResult MouseDown(const PosInfo&, Points&) = 0;
  virtual PtResult MouseUp(const PosInfo&, Points&) = 0;
  virtual Cursor GetCursor() const = 0;
  virtual bool Change(PointType newType, Points&) = 0;
  virtual Point GetCurrentPos() const = 0;
};

static PathPt bezier_line(const Point& from, const Point& to){
  Point ctrl(from.x + (to.x - from.x) / 2,
    from.y + (to.y - from.y) / 2);
  return PathPt::CubicBezierTo(to, ctrl, ctrl);
}

class CurvePointBuilder : public PointBuilder{
private:
  enum States { FIRST_POINT, CONTROL_1, CONTROL_2 };
  const Point m_from;
  Point m_currentPos;
  MouseButton m_mouseButton;
  States m_state;
public:
  CurvePointBuilder(const Point& end, Points& path, MouseButton mouseButton)
    : m_from(path.Back().p),
      m_mouseButton(mouseButton),
      m_state(FIRST_POINT)
  {
    path.Append(bezier_line(m_from, end));
    m_currentPos = end;
  }

  PtResult Motion(const PosInfo& info, Points& path) override{
    const Point& pt = info.pos;
    m_currentPos = pt;
    UpdateStatus(info.status);
    if (m_state == FIRST_POINT){
      path.AdjustBack(bezier_line(m_from, pt));
    }
    else if (m_state == CONTROL_1){
      PathPt prev(path.Back());
      // While editing the first control point, move also the second
      // control point
      prev.c = prev.d = pt;
      path.AdjustBack(prev);
    }
    else if (m_state == CONTROL_2){
      PathPt prev(path.Back());
      prev.c = pt;
      path.AdjustBack(prev);
    }
    return PtResult::DRAW;
  }

  PtResult MouseDown(const PosInfo& info, Points& path) override{
    bool sameButton = info.modifiers.MouseButton() == m_mouseButton;
    if (m_state == FIRST_POINT){
      m_state = CONTROL_1;
      return sameButton ? PtResult::DRAW :
        PtResult::PATH_DONE;
    }
    else if (m_state == CONTROL_1){
      m_state = CONTROL_2;
      Motion(info, path);
      return sameButton ? PtResult::DRAW :
        PtResult::PATH_DONE;
    }
    else if (m_state == CONTROL_2){
      return sameButton ? PtResult::POINT_DONE :
        PtResult::PATH_DONE;
    }
    assert(false);
    return PtResult::NONE;
  }

  PtResult MouseUp(const PosInfo&, Points&) override{
    return PtResult::NONE;
  }

  Cursor GetCursor() const override{
    switch (m_state){
    case FIRST_POINT:
      return Cursor::CURVE;
    case CONTROL_1:
      return Cursor::CONTROL_POINT_1;
    case CONTROL_2:
      return Cursor::CONTROL_POINT_2;
    default:
      assert(false);
      return Cursor::CURVE;
    }
  }

  bool Change(PointType newType, Points& path) override{
    if (newType != PointType::CURVE){
      path.PopBack();
      return true;
    }
    return false;
  }

  virtual Point GetCurrentPos() const override{
    return m_currentPos;
  };

  CurvePointBuilder& operator=(const CurvePointBuilder&) = delete;

private:
  void UpdateStatus(StatusInterface& status){
    StrBtn btn(m_mouseButton);
    switch (m_state){
    case FIRST_POINT:
      status.SetMainText(space_sep(btn.This(true), "click to add curve,",
        btn.Other(true),
        "click to stop."));
      break;
    case CONTROL_1:
      status.SetMainText(space_sep(btn.This(true),
        "click to place first control point,",
        btn.Other(true),
        "click to stop."));
      break;
    case CONTROL_2:
      status.SetMainText(space_sep(btn.This(true),
        "click to place the second control point,",
        btn.Other(true),
        "click to stop."));
      break;
    }
  }
};

class MovePointBuilder : public PointBuilder{
  PtResult Motion(const PosInfo& info, Points&) override{
    info.status.SetMainText("Click to start drawing a path.");
    return PtResult::NONE;
  }

  PtResult MouseDown(const PosInfo& info, Points& path) override{
    path.Append(PathPt::MoveTo(info.pos));
    return PtResult::POINT_DONE;
  }

  PtResult MouseUp(const PosInfo&, Points&) override{
    return PtResult::NONE;
  }

  Cursor GetCursor() const override{
    return Cursor::CROSSHAIR;
  }

  bool Change(PointType, Points&) override{
    // The MovePointBuilder should not be replaced with e.g. line or
    // curve, an initial point must be added.
    return false;
  }

  Point GetCurrentPos() const override{
    return Point(0,0);
  }
};

class LinePointBuilder : public PointBuilder{
public:
  Point m_currentPos;
  LinePointBuilder(const Point& endPoint, Points& path, MouseButton button)
    : m_mouseButton(button)
  {
    path.Append(PathPt::LineTo(endPoint));
    m_currentPos = endPoint;
  }

  PtResult Motion(const PosInfo& info, Points& path) override{
    UpdateStatus(info.status);
    path.AdjustBack(PathPt::LineTo(info.pos));
    m_currentPos = info.pos;
    return PtResult::DRAW;
  }

  PtResult MouseDown(const PosInfo& info, Points&) override{
    return info.modifiers.MouseButton() == m_mouseButton ?
      PtResult::POINT_DONE :
      PtResult::PATH_DONE;
  }

  PtResult MouseUp(const PosInfo&, Points&) override{
    return PtResult::DRAW;
  }

  Cursor GetCursor() const override{
    return Cursor::BRUSH;
  }

  bool Change(PointType newType, Points& path) override{
    if (newType != PointType::LINE){
      path.PopBack();
      return true;
    }
    return false;
  }

  Point GetCurrentPos() const override{
    return m_currentPos;
  }
private:
  void UpdateStatus(StatusInterface& status){
    StrBtn btn(m_mouseButton);
    status.SetMainText(space_sep(btn.This(true), "click to add line,",
        btn.Other(true), "click to stop"));
  }
  MouseButton m_mouseButton;
};

Settings path_tool_settings(const Settings& allSettings){
  Settings s(default_path_settings());
  s.Set(ts_PointType, PointType::LINE);
  s.Set(ts_AntiAlias, true);
  s.Set(ts_ClosedPath, true);
  s.Update(allSettings);
  return s;
}

class PathTool : public Tool{
  // The PathTool is not a StandardTool because it needs to react to
  // setting changes.
public:
  PathTool(const Settings& allSettings)
    : Tool(ToolId::PATH),
      m_active(false),
      m_mouseButton(MouseButton::LEFT),
      m_pointBuilder(new MovePointBuilder()),
      m_settings(path_tool_settings(allSettings))
  {}

  ToolResult Char(const KeyInfo& info) override{
    if (m_active && info.key.Is(key::esc)){
      // Commit a drawn path on escape (though the preferred method is
      // right click)
      return Commit(info.layerType);
    }
    return ToolResult::DRAW;
  }

  void Draw(FaintDC& dc, Overlays&, const PosInfo&) override{
    if (m_active && m_path.Size() >= 2){
      std::vector<PathPt> points = m_path.GetPoints();
      if (m_settings.Get(ts_ClosedPath)){
        points.push_back(PathPt::PathCloser());
      }
      dc.Path(points, m_settings);
    }
  }

  bool DrawBeforeZoom(Layer layer) const override{
    return layer == Layer::RASTER;
  }

  bool EatsSettings() const override{
    return false;
  }

  Command* GetCommand() override{
    return m_command.Take();
  }

  Cursor GetCursor(const PosInfo&) const override{
    return m_pointBuilder->GetCursor();
  }

  IntRect GetRefreshRect(const RefreshInfo&) const override{
    // \ref(hack-miter-growth)
    const coord inflateFactor = 10.0 * m_settings.Get(ts_LineWidth);
    return floored(inflated(bounding_rect(m_path.GetTri()),  inflateFactor));
  }

  const Settings& GetSettings() const override{
    return m_settings;
  }

  Optional<const faint::HistoryContext&> HistoryContext() const override{
    return {};
  }

  ToolResult DoubleClick(const PosInfo& info) override{
    if (m_active){
      return HandlePtResult(m_pointBuilder->MouseDown(info, m_path), info);
    }
    return ToolResult::NONE;
  }

  ToolResult MouseDown(const PosInfo& info) override{
    if (!m_active){
      m_mouseButton = info.modifiers.MouseButton();
      m_active = true;
    }
    return HandlePtResult(m_pointBuilder->MouseDown(info, m_path), info);
  }

  ToolResult MouseUp(const PosInfo& info) override{
    return HandlePtResult(m_pointBuilder->MouseUp(info, m_path), info);
  }

  ToolResult MouseMove(const PosInfo& info) override{
    return HandlePtResult(m_pointBuilder->Motion(info, m_path), info);
  }

  ToolResult Preempt(const PosInfo& info) override{
    if (m_active){
      return Commit(info.layerType);
    }
    return ToolResult::NONE;
  }

  bool RefreshOnMouseOut() const override{
    return false;
  }

  void SelectionChange() override{
  }

  Optional<const faint::SelectionContext&> SelectionContext() const override{
    return {};
  }

  bool Set(const BoundSetting& s) override{
    bool changed = m_settings.Update(s);
    auto pointType = m_settings.Get(ts_PointType);
    if (changed && m_pointBuilder->Change(pointType, m_path)){
      m_pointBuilder = CreatePointBuilder(m_pointBuilder->GetCurrentPos());
    }
    return changed;
  }

  void SetLayer(Layer) override{
  }

  Optional<const faint::TextContext&> TextContext() const override{
    return {};
  }

  bool UpdateSettings(const Settings& s) override{
    bool changed = m_settings.Update(s);
    auto pointType = m_settings.Get(ts_PointType);
    if (changed && m_pointBuilder->Change(pointType, m_path)){
      m_pointBuilder = CreatePointBuilder(m_pointBuilder->GetCurrentPos());
    }
    return changed;
  }

private:
  ToolResult HandlePtResult(PtResult result, const PosInfo& info){
    switch(result){
    case PtResult::NONE:
      return ToolResult::NONE;
    case PtResult::DRAW:
      return ToolResult::DRAW;
    case PtResult::POINT_DONE:
      m_pointBuilder = CreatePointBuilder(info.pos);
      return ToolResult::DRAW;
    case PtResult::PATH_DONE:
      return Commit(info.layerType);
    default:
      assert(false);
      return ToolResult::NONE;
    }
  }

  ToolResult Commit(Layer layer){
    if (m_settings.Get(ts_ClosedPath)){
      m_path.Append(PathPt::PathCloser());
    }
    m_command.Set(add_or_draw(create_path_object(m_path,
      without(m_settings, ts_PointType, ts_ClosedPath)),
      layer));
    m_active = false;
    m_path.Clear();
    m_pointBuilder = std::make_unique<MovePointBuilder>();
    return ToolResult::COMMIT;
  }

  std::unique_ptr<PointBuilder> CreatePointBuilder(const Point& pos){
    PointType t = m_settings.Get(ts_PointType);
    if (t == PointType::LINE){
      return std::make_unique<LinePointBuilder>(pos, m_path, m_mouseButton);
    }
    else{
      return std::make_unique<CurvePointBuilder>(pos, m_path, m_mouseButton);
    }
  }

  bool m_active;
  PendingCommand m_command;
  MouseButton m_mouseButton;
  std::unique_ptr<PointBuilder> m_pointBuilder;
  Points m_path;
  Settings m_settings;
};

Tool* path_tool(const Settings& allSettings){
  return new PathTool(allSettings);
}

} // namespace
