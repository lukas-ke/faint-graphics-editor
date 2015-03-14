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
#include "bitmap/alpha-map.hh"
#include "bitmap/brush.hh"
#include "bitmap/color.hh"
#include "commands/command.hh"
#include "geo/adjust.hh"
#include "geo/geo-func.hh"
#include "geo/int-rect.hh"
#include "geo/line.hh"
#include "geo/padding.hh"
#include "geo/measure.hh"
#include "rendering/faint-dc.hh"
#include "rendering/overlay.hh"
#include "rendering/render-brush.hh"
#include "text/formatting.hh"
#include "tools/standard-tool.hh"
#include "util/pos-info.hh"
#include "util/setting-util.hh"
#include "util/tool-util.hh"

namespace faint{

static bool eraser(const PosInfo& info){
  return info.tabletCursor == TABLET_CURSOR_ERASER;
}

static bool translucent(const Brush& b){
  const IntSize sz(b.GetSize());
  for (int y = 0; y != sz.h; y++){
    for (int x = 0; x != sz.w; x++){
      uchar a = b.Get(x,y);
      if (a != 0 && a != 255){
        return true;
      }
    }
  }
  return false;
}

class BrushCommand : public Command{
public:
  BrushCommand(const UpperLeft& topLeft,
    const AlphaMap& alphaMap,
    const UpperLeft& first,
    const Settings& settings)
    : Command(CommandType::RASTER),
      m_settings(settings),
      m_alphaMap(alphaMap),
      m_first(first.Get()),
      m_topLeft(topLeft.Get())
  {
    finalize_swap_colors_erase_bg(m_settings);
  }

  utf8_string Name() const{
    return "Brush Stroke";
  }

  void Do(CommandContext& context){
    context.GetDC().Blend(m_alphaMap, m_topLeft, m_first, m_settings);
  }

private:
  Settings m_settings;
  AlphaMap m_alphaMap;
  IntPoint m_first;
  IntPoint m_topLeft;
};

static Settings brush_settings(const Settings& allSettings){
  Settings s;
  s.Set(ts_BrushSize, 1);
  s.Set(ts_BrushShape, BrushShape::SQUARE);
  s.Set(ts_Fg, Paint(Color(0,0,0)));
  s.Set(ts_Bg, Paint(Color(0,0,0)));
  s.Set(ts_SwapColors, false);
  s.Set(ts_Filter, 0);
  s.Update(allSettings);
  return s;
}

static std::vector<LineSegment> floated(const std::vector<IntLineSegment>& v){
  std::vector<LineSegment> out;
  out.reserve(v.size());
  for (const auto& l : v){
    out.push_back(floated(l));
  }
  return out;
}

static Settings maybe_offsat_paint(const Settings& s, const Brush& brush){
  ColorSetting which = (s.Has(ts_SwapColors) && s.Get(ts_SwapColors)) ?
    ts_Bg : ts_Fg;
  Settings s2(s);
  s2.Set(which, offsat_if_object_aligned(s.Get(which),
    -point_from_size(brush.GetSize() / 2)));
  return s2;
}

class BrushTool : public StandardTool {
public:
  BrushTool(const Settings& allSettings)
    : StandardTool(ToolId::BRUSH, brush_settings(allSettings)),
      m_alphaMap(IntSize(1, 1)),
      m_brush(circle_brush(1)),
      m_brushOverlay(IntSize(1,1)),
      m_constrainDir(ConstrainDir::NONE),
      m_first(IntPoint()),
      m_prev(IntPoint()),
      m_translucent(false)
  {
    m_active = false;

    // Prevent cursor being drawn at 0,0 before motion.
    m_drawCursor = false; // Todo: Still necessary?
  }

  void Draw(FaintDC& dc, Overlays& overlays, const PosInfo& info) override{
    const Point& p(info.pos);

    if (m_active){
      dc.Blend(m_alphaMap, IntPoint(0,0), m_first.Get(),
        maybe_offsat_paint(GetSettings(), m_brush));
    }
    else if (m_drawCursor){
      dc.Blend(m_brushOverlay, brush_top_left(p, m_brush).Get(), floored(p),
        GetSettings());
    }

    if (m_drawCursor && m_translucent){
      // Show the brush boundaries as a thin outline
      overlays.Lines(m_brushEdge, brush_top_left(p, m_brush).Get());
    }
  }

  bool DrawBeforeZoom(Layer) const override{
    return true;
  }

  Command* GetCommand() override{
    return m_command.Take();
  }

  Cursor GetCursor(const PosInfo& info) const override{
    if (outside_canvas_by(info, GetSettings().Get(ts_BrushSize) / 2)){
      return Cursor::BRUSH_OUT;
    }
    return Cursor::BRUSH;
  }

  IntRect GetRefreshRect(const RefreshInfo& info) const override{
    if (!m_active){
      const IntPoint p(floored(info.mousePos));
      return padded(IntRect(p, p), get_padding(GetSettings()));
    }
    return info.visibleRect;
  }

  ToolResult MouseDown(const PosInfo& info) override{
    Reset(info);
    m_prev = brush_top_left(info.pos, m_brush);
    m_covered = floored(Rect(info.pos, info.pos));
    m_origin = info.pos;
    if (info.modifiers.RightMouse() || eraser(info)){
      // Test: Color swap on digitizer eraser end
      SetSwapColors(true);
    }
    else{
      SetSwapColors(false);
    }
    stroke(m_alphaMap, m_prev, m_prev, m_brush);

    return ToolResult::DRAW;
  }

  ToolResult MouseUp(const PosInfo&) override{
    if (!m_active){
      return ToolResult::NONE;
    }
    return Commit();
  }

  ToolResult MouseMove(const PosInfo& info) override{
    info.status.SetMainText("");
    info.status.SetText(str(info.pos), 0);

    Point pos(info.pos);
    if (!m_drawCursor){
      InitBrush();
      m_drawCursor = true;
    }

    if (m_active) {
      const bool constrainHeld(info.modifiers.Secondary());
      if (constrainHeld) {
        if (m_constrainDir == ConstrainDir::NONE){
          if (distance(pos, m_origin) > 5){
            // Lock the constrain direction after a certain distance
            m_constrainDir = constrain_pos(pos, m_origin);
          }
          else{
            // Don't draw anything until the constrain direction is
            // ascertained, to avoid "bumps".
            return ToolResult::NONE;
          }
        }
        else{
          constrain_pos(pos, m_origin, m_constrainDir);
        }
      }
      else {
        m_origin = pos;
        m_constrainDir = ConstrainDir::NONE;
      }

      UpperLeft newPos = brush_top_left(pos, m_brush);
      stroke(m_alphaMap, m_prev, newPos, m_brush);
      m_covered = bounding_rect(m_covered.TopLeft(),
        m_covered.BottomRight(),
        floored(pos));
      m_prev = newPos;
      return ToolResult::DRAW;
    }
    else{
      // Test: Swap brush cursor color on digitizer eraser end
      SetSwapColors(info.tabletCursor == TABLET_CURSOR_ERASER);
    }
    return ToolResult::DRAW;
  }

  ToolResult Preempt(const PosInfo&) override{
    return ToolResult::NONE;
  }

  bool RefreshOnMouseOut() const override{
    // Prevent leaving a brush cursor dropping when mouse leaves the
    // window
    m_drawCursor = false;
    return true;
  }

private:
  void InitBrush(){
    m_brush = get_brush(GetSettings());
    m_brushEdge = floated(brush_edge(m_brush));
    init_brush_overlay(m_brushOverlay, m_brush);
    m_translucent = translucent(m_brush);
  }

  ToolResult Commit(){
    assert(m_active);
    m_active = false;

    IntRect r(intersection(padded(m_covered, get_padding(GetSettings())),
      IntRect(IntPoint(0,0), m_alphaMap.GetSize())));

    if (empty(r)){
      return ToolResult::NONE;
    }

    m_command.Set(new BrushCommand(r.TopLeft(),
      m_alphaMap.SubCopy(r),
      m_first, maybe_offsat_paint(GetSettings(), m_brush)));
    return ToolResult::COMMIT;
  }

  void Reset(const PosInfo& info){
    m_active = true;
    m_alphaMap.Reset(info.canvas.GetSize());
    m_constrainDir = ConstrainDir::NONE;
    InitBrush();

    m_first = brush_top_left(info.pos, m_brush);
  }
  bool m_active;
  AlphaMap m_alphaMap;
  Brush m_brush;
  std::vector<LineSegment> m_brushEdge;
  AlphaMap m_brushOverlay;
  PendingCommand m_command;
  ConstrainDir m_constrainDir;
  mutable bool m_drawCursor;
  UpperLeft m_first;
  Point m_origin;
  UpperLeft m_prev;
  IntRect m_covered;
  bool m_translucent;
};

Tool* brush_tool(const Settings& s){
  return new BrushTool(s);
}

} // namespace
