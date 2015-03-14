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
#include "geo/adjust.hh"
#include "geo/geo-func.hh"
#include "geo/int-rect.hh"
#include "geo/line.hh"
#include "geo/points.hh"
#include "objects/objpolygon.hh"
#include "rendering/faint-dc.hh"
#include "text/formatting.hh"
#include "tools/standard-tool.hh"
#include "util/command-util.hh"
#include "util/container-util.hh"
#include "util/default-settings.hh"
#include "util/grid.hh"
#include "util/object-util.hh"
#include "util/pos-info.hh"
#include "util/undo-redo.hh"
#include "util-wx/key-codes.hh"

namespace faint{

static bool should_snap(const ToolModifiers& modifiers){
  return modifiers.OnlyPrimary();
}

static std::vector<Point> get_point_intersections(const Points& in_points,
  int pointIndex)
{
  std::vector<Point> points(in_points.GetPointsDumb());
  int numPoints = resigned(points.size());
  if (numPoints <= 1){
    return std::vector<Point>();
  }
  if (pointIndex >= numPoints){
    return std::vector<Point>();
  }
  assert(pointIndex < numPoints);
  const bool firstPoint = pointIndex == 0;
  const bool lastPoint = pointIndex == numPoints - 1;
  Point p0 = firstPoint ? get_item(points, -1) :
    get_item(points, pointIndex - 1);
  Point p1 = lastPoint ? get_item(points, 0) :
    get_item(points, pointIndex + 1);

  // Diagonal to diagonal
  const double m0 = p0.y - p0.x;
  const double m1 = p1.y + p1.x;
  const double x0 = (m0 + m1) / 2.0 - m0;
  const double y0 = x0 + m0;
  Point snapPt(x0, y0);

  return {
    Point(p0.x, p1.y), // Corner
    Point(p1.x, p0.y), // Corner
    Point(p1.y - (p0.y - p0.x), p1.y), // Diagonal to horizontal
    Point((p0.y + p0.x) - p1.y, p1.y), // Diagonal to horizontal

    Point(p0.y - (p1.y - p1.x), p0.y), // Diagonal to horizontal
    Point((p1.y + p1.x) - p0.y, p0.y), // Diagonal to horizontal

    Point(p1.x, p1.x + (p0.y - p0.x)), // Diagonal to vertical
    Point(p1.x, -p1.x + (p0.y + p0.x)), // Diagonal to vertical

    Point(p0.x, p0.x + (p1.y - p1.x)), // Diagonal to vertical
    Point(p0.x, -p0.x + (p1.y + p1.x)), // Diagonal to vertical
    snapPt,
    p1 + (p0 - snapPt)};
}

static Point polygon_snap(const Point& pos, Canvas& canvas, const Points& points){
  return snap(pos, canvas.GetObjects(), canvas.GetGrid(),
    get_point_intersections(points, points.Size() - 1));
}

static Settings polygon_tool_settings(const Settings& allSettings){
  Settings s(default_polygon_settings());
  s.Update(allSettings);
  return s;
}

  class PolygonTool : public StandardTool, public HistoryContext {
public:
  PolygonTool(const Settings& allSettings)
    : StandardTool(ToolId::POLYGON, polygon_tool_settings(allSettings)),
      m_active(false),
      m_mouseButton(MouseButton::LEFT)
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

  ToolResult Char(const KeyInfo& info) override{
    if (m_active && info.key.Is(key::esc)){
      // Commit a drawn polygon on escape (though the preferred method
      // is right click)
      return Commit(info.layerType);
    }
    return ToolResult::NONE;
  }

  void Draw(FaintDC& dc, Overlays&, const PosInfo&) override{
    if (m_active && m_points.Size() >= 2){
      dc.Polygon(m_points.GetTri(), m_points.GetPointsDumb(), GetSettings());
    }
  }

  bool DrawBeforeZoom(Layer layer) const override{
    return layer == Layer::RASTER;
  }

  Command* GetCommand() override{
    return m_command.Take();
  }

  Cursor GetCursor(const PosInfo&) const override{
    return Cursor::SQUARE_CROSS;
  }

  utf8_string GetRedoName() const override{
    return "Add Point";
  }

  IntRect GetRefreshRect(const RefreshInfo&) const override{
    // \def(hack-miter-growth)Fixme: Kind of hackily grow more than linewidth;
    // to account for e.g. miter-join protrusions, and it doesn't even work.
    const coord inflateFactor = 10.0 * GetSettings().Get(ts_LineWidth);
    return floored(inflated(bounding_rect(m_points.GetTri()), inflateFactor));
  }

  utf8_string GetUndoName() const override{
    return "Add Point";
  }

  Optional<const faint::HistoryContext&> HistoryContext() const override{
    return Optional<const faint::HistoryContext&>(*this);
  }

  ToolResult DoubleClick(const PosInfo&) override{
    return ToolResult::NONE;
  }

  ToolResult MouseDown(const PosInfo& info) override{
    SetAntiAlias(info);
    Point pos = info.pos;
    if (should_snap(info.modifiers)){
      pos = polygon_snap(pos, info.canvas, m_points);
    }

    MouseButton mouseButton = info.modifiers.MouseButton();
    if (!m_active){
      m_active = true;
      SetSwapColors(info.modifiers.RightMouse());

      // Add the first point twice so that the polygon always contains
      // at least two points.
      m_points.Append(pos);
      return AddPoint(pos, mouseButton);
    }
    else {
      // Can only adjust to 45-degrees from the second point on,
      // i.e. if we're already active
      pos = ConstrainPoint(pos, info.modifiers);
    }

    if (mouseButton == m_mouseButton){
      m_points.AdjustBack(pos);
      return AddPoint(pos, mouseButton);
    }
    else {
      // Switched mouse button means commit
      return Commit(info.layerType);
    }
  }

  ToolResult MouseUp(const PosInfo&) override{
    return ToolResult::NONE;
  }

  ToolResult MouseMove(const PosInfo& info) override{
    if (m_points.Size() >= 2){
      info.status.SetText(str_line_status_subpixel(
        {m_points.BaBack().p, m_points.Back().p}), 0);
    }
    else{
      info.status.SetText(str(info.pos), 0);
    }

    if (!m_active){
      info.status.SetMainText("Click to start drawing a polygon.");
    }
    else {
      SetAntiAlias(info);
      StrBtn btn(m_mouseButton);
      info.status.SetMainText(space_sep(btn.This(true), "click to add points,",
        btn.Other(true), "click to stop."));
    }
    if (!m_active || m_points.Size() < 1){
      return ToolResult::NONE;
    }

    Point pos = ConstrainPoint(info.pos, info.modifiers);
    if (should_snap(info.modifiers)) {
      pos = polygon_snap(pos, info.canvas, m_points);
    }

    m_points.AdjustBack(pos);
    return ToolResult::DRAW;
  }

  ToolResult Preempt(const PosInfo& info) override{
    if (m_active){
      return Commit(info.layerType);
    }
    return ToolResult::NONE;
  }

  void Redo() override{
    m_points.PopBack();
    Point redone = m_states.Redo();
    m_points.Append(redone);
    m_points.Append(redone); // Cursor pos
  }

  void Undo() override{
    assert(m_states.CanUndo());
    m_points.PopBack();
    m_states.Undo();
  }
private:
  ToolResult AddPoint(const Point& pos, MouseButton button){
    m_points.Append(pos);
    m_states.Did(pos);
    m_mouseButton = button;
    return ToolResult::DRAW;
  }

  ToolResult Commit(Layer layer){
    m_command.Set(add_or_draw(create_polygon_object(m_points, GetSettings()),
      layer));
    Reset();
    return ToolResult::COMMIT;
  }

  Point ConstrainPoint(const Point& p, const ToolModifiers& modifiers){
    if (!modifiers.Secondary()){
      return p;
    }

    const std::vector<Point> pts = m_points.GetPointsDumb();

    // Constrain relative to the first point if both modifiers used
    const Point oppositePos = modifiers.Primary() ?
      get_item(pts, 0) :
      get_item(pts, -2);

    return adjust_to_45(oppositePos, p);
  }

  void Reset(){
    m_active = false;
    m_points.Clear();
    m_states.Clear();
  }

  Points m_points;
  UndoRedo<Point> m_states;
  bool m_active;
  MouseButton m_mouseButton;
  PendingCommand m_command;
};

Tool* polygon_tool(const Settings& allSettings){
  return new PolygonTool(allSettings);
}

} // namespace
