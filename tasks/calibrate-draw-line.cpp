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
#include "editors/line-editor.hh"
#include "geo/geo-func.hh"
#include "geo/int-rect.hh"
#include "geo/line.hh"
#include "geo/measure.hh"
#include "geo/rect.hh"
#include "rendering/overlay.hh"
#include "tasks/calibrate-tasks.hh"
#include "tasks/standard-task.hh"
#include "util/pos-info.hh"

namespace faint{

class CalibrateDrawLine : public StandardTask{
  // Task for drawing a line for calibrating the image coordinate
  // system.
public:
  CalibrateDrawLine(const PosInfo& info)
    : m_editor(info, AllowConstrain(true), AllowSnap(true))
  {
    info.status.SetMainText("Draw measuring line.");
  }

  virtual void Draw(FaintDC&, Overlays& overlays, const PosInfo&) override{
    overlays.Line(m_editor.GetLine());
  }

  virtual bool DrawBeforeZoom(Layer) const override{
    return false;
  }

  Command* GetCommand() override{
    return nullptr;
  }

  Cursor GetCursor(const PosInfo&) const override{
    return Cursor::CALIBRATE_CROSSHAIR;
  }

  Task* GetNewTask() override{
    return m_newTask.Take();
  }

  IntRect GetRefreshRect(const RefreshInfo&) const override{
    return m_editor.GetRefreshRect();
  };

  TaskResult MouseDown(const PosInfo&) override{
    // The CalibrateDrawLine task is created on MouseDown, and
    // terminates on MouseUp.
    return TaskResult::NONE;
  }

  TaskResult MouseUp(const PosInfo& info) override{
    m_editor.MouseUp(info);
    default_line_status(info, m_editor);

    auto line(m_editor.GetLine());
    if (length(line) != 0){
      m_newTask.Set(calibrate_enter_measure(line, info));
    }
    return TaskResult::CHANGE;
  }

  TaskResult MouseMove(const PosInfo& info) override{
    m_editor.MouseMove(info);
    default_line_status(info, m_editor);
    return TaskResult::DRAW;
  }

  TaskResult Preempt(const PosInfo&) override{
    return TaskResult::CHANGE;
  }

private:
  LineEditor m_editor;
  PendingTask m_newTask;
};

Task* calibrate_draw_line(const PosInfo& info){
  return new CalibrateDrawLine(info);
}

} // namespace
