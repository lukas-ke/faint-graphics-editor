// -*- coding: us-ascii-unix -*-
// Copyright 2015 Lukas Kemmer
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

#include "editors/line-editor.hh"
#include "geo/angle.hh"
#include "geo/calibration.hh"
#include "geo/geo-func.hh"
#include "geo/line.hh"
#include "geo/measure.hh"
#include "geo/size.hh"
#include "geo/tri.hh"
#include "rendering/faint-dc.hh"
#include "rendering/overlay.hh"
#include "text/formatting.hh"
#include "text/text-expression-conversions.hh"
#include "tools/standard-tool.hh"
#include "util/default-settings.hh"
#include "util/pos-info.hh"

namespace faint{

static Settings measure_settings(const Settings& s){
  Settings s2;
  s2.Set(ts_Unit, s.Get(ts_Unit));
  s2.Set(ts_TapeStyle, s.Get(ts_TapeStyle));
  return s2;
}

static Tri get_tape_text_tri(const LineSegment& l){
  return {mid_point(l), Angle::Zero(), Size(100,100)};
}

class TapeMeasureTool : public StandardTool {
public:
  TapeMeasureTool(const Settings& allSettings)
    : StandardTool(ToolId::TAPE_MEASURE, measure_settings(allSettings)),
      m_editor(AllowConstrain(true), AllowSnap(true))
  {}

  void Draw(FaintDC&, Overlays& overlays, const PosInfo& info) override{
    if (!m_active){
      return;
    }

    // Fixme: Simplify

    auto line = m_editor.GetLine();
    overlays.Line(line);

    const auto& settings = GetSettings();

    auto& conversions = length_conversions();
    utf8_string unit = settings.Get(ts_Unit);
    Calibration c = get_calibration(info).Or(Calibration());
    if (c.unit.empty()){
      unit = "px";
    }

    coord sc = 1.0;
    if (unit == "px"){
      sc = 1.0;
    }
    else {
      auto it = conversions.find(unit);
      if (it == end(conversions)){
        sc = 1.0;
      }
      else{
        it->second[c.unit].Visit(
          [&](coord calibratedToWanted){
            sc = calibratedToWanted / c.Scale();
          });
      }
    }

    overlays.Text(mid_point(line),
      space_sep(str(length(line) * sc, 2_dec), unit));

    if (settings.Get(ts_TapeStyle) == TapeMeasureStyle::LINE){
      return;
    }

    Rect r(bounding_rect(line));
    if (line.p0.x < line.p1.x){
      if (line.p0.y < line.p1.y){
        auto left = left_side(r);
        auto bottom = bottom_side(r);
        overlays.Line(left);
        overlays.Line(bottom);
        overlays.Text(mid_point(bottom),
          space_sep(str(length(bottom) * sc, 2_dec), unit));
        overlays.Text(mid_point(left),
          space_sep(str(length(left) * sc, 2_dec), unit));
      }
      else{
        auto left = left_side(r);
        auto top = top_side(r);
        overlays.Line(left);
        overlays.Line(top);
        overlays.Text(mid_point(left),
          space_sep(str(length(left) * sc, 2_dec), unit));
        overlays.Text(mid_point(top),
          space_sep(str(length(top) * sc, 2_dec), unit));
      }
    }
    else{
      if (line.p0.y < line.p1.y){
        auto left = left_side(r);
        auto top = top_side(r);
        overlays.Line(left);
        overlays.Line(top);
        overlays.Text(mid_point(left),
          space_sep(str(length(left) * sc, 2_dec), unit));
        overlays.Text(mid_point(top),
          space_sep(str(length(top) * sc, 2_dec), unit));
      }
      else{
        auto right = right_side(r);
        auto top = top_side(r);
        overlays.Line(right);
        overlays.Line(top);
        overlays.Text(mid_point(right),
          space_sep(str(length(right) * sc, 2_dec), unit));
        overlays.Text(mid_point(top),
          space_sep(str(length(top) * sc, 2_dec), unit));
      }
    }
  }

  bool DrawBeforeZoom(Layer) const override{
    return false;
  }

  Command* GetCommand() override{
    return nullptr;
  }

  Cursor GetCursor(const PosInfo&) const override{
    return Cursor::CROSSHAIR;
  }

  IntRect GetRefreshRect(const RefreshInfo&) const override{
    const auto line(m_editor.GetLine());
    return floiled(union_of(bounding_rect(line),
        bounding_rect(get_tape_text_tri(line))));
  }

  ToolResult MouseDown(const PosInfo& info) override{
    if (!m_active){
      m_active = true;
      m_editor.MouseDown(info);
      default_line_status(info, m_editor);
    }
    return ToolResult::DRAW;
  }

  ToolResult MouseMove(const PosInfo& info) override{
    if (!m_active){
      info.status.SetMainText("");
      info.status.SetText(str(info.pos), 0);
      return ToolResult::NONE;
    }
    else{
      m_editor.MouseMove(info);
      default_line_status(info, m_editor);
    }
    return ToolResult::DRAW;
  }

  ToolResult MouseUp(const PosInfo& info) override{
    if (m_active){
      m_active = false;
      m_editor.MouseUp(info);
      default_line_status(info, m_editor);
    }
    return ToolResult::DRAW;
  }

  ToolResult Preempt(const PosInfo&) override{
    if (m_active){
      m_active = false;
      return ToolResult::CANCEL;
    }
    return ToolResult::NONE;
  }

private:
  bool m_active = false;
  LineEditor m_editor;
};

Tool* tape_measure_tool(const Settings& allSettings){
  return new TapeMeasureTool(allSettings);
}

} // namespace
