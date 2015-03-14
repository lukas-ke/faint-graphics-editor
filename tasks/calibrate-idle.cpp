// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
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

#include "app/resource-id.hh"
#include "geo/measure.hh"
#include "geo/rect.hh"
#include "rendering/overlay.hh"
#include "tasks/calibrate-tasks.hh"
#include "tasks/standard-task.hh"
#include "text/formatting.hh"
#include "util/pos-info.hh"

namespace faint{

static bool on_line_end(const LineSegment& l, const Point& pos){
  return
    inflated(Rect(l.p0, l.p0), 5.0).Contains(pos) ||
    inflated(Rect(l.p1, l.p1), 5.0).Contains(pos);
}

class CalibrateIdle : public StandardTask{
  // The idle state of the CalibrateTool, before clicking to draw a
  // line. Shows the current calibration (if any) as an overlay.
public:
  void Draw(FaintDC&, Overlays& overlays, const PosInfo& info) override{
    const auto& c = get_calibration(info);
    c.Visit(
      [&](const Calibration& c){
        overlays.Line(c.pixelLine);
        overlays.Text(mid_point(c.pixelLine),
          space_sep(str(length(c.pixelLine) / c.Scale(), 2_dec), c.unit));
      });
  }

  bool DrawBeforeZoom(Layer) const override{
    return false;
  }

  Command* GetCommand() override{
    return nullptr;
  }

  Cursor GetCursor(const PosInfo& info) const override{
    return get_calibration(info).Visit(
      [pos = info.pos](const Calibration& c){
        return on_line_end(c.pixelLine, pos) ?
          Cursor::MOVE_POINT :
          Cursor::CALIBRATE_CROSSHAIR;
      },
      [](){
        return Cursor::CALIBRATE_CROSSHAIR;
      });
  }


  Task* GetNewTask() override{
    return m_newTask.Take();
  }

  IntRect GetRefreshRect(const RefreshInfo&) const override{
    return {};
  }

  TaskResult MouseDown(const PosInfo& info) override{
    m_newTask.Set(calibrate_draw_line(info));
    return TaskResult::CHANGE;
  }

  TaskResult MouseMove(const PosInfo&) override{
    return TaskResult::NONE;
  }

  TaskResult MouseUp(const PosInfo&) override{
    return TaskResult::NONE;
  }

  TaskResult Preempt(const PosInfo&) override{
    return TaskResult::NONE;
  }

private:
  PendingTask m_newTask;
};

Task* calibrate_idle(){
  return new CalibrateIdle();
}

} // namespace
