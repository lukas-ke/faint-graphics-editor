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

#include <algorithm>
#include "geo/geo-func.hh"
#include "geo/int-point.hh"
#include "geo/measure.hh"
#include "geo/rect.hh"
#include "rendering/overlay.hh"
#include "tasks/standard-task.hh"
#include "tasks/text-edit.hh"
#include "tasks/text-make-box.hh"
#include "text/formatting.hh"
#include "util/pos-info.hh"

namespace faint{

class MakeTextBox : public StandardTask {
public:
  MakeTextBox(const Point& startPos, Settings& s)
    : m_maxDistance(0.0),
      m_p0(startPos),
      m_p1(startPos),
      m_settings(s)
  {}

  void Draw(FaintDC&, Overlays& overlays, const PosInfo&) override{
    overlays.Rectangle(Rect(m_p0, m_p1));
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

  Task* GetNewTask() override{
    return m_newTask.Take();
  }

  IntRect GetRefreshRect(const RefreshInfo&) const override{
    return IntRect(floored(m_p0), floored(m_p1));
  }

  TaskResult MouseDown(const PosInfo&) override{
    // Rectangle selection is already active after construction
    return TaskResult::NONE;
  }

  TaskResult MouseUp(const PosInfo&) override{
    // Use free form text, instead of bounded by a rectangle, if the
    // drawn box is small.
    m_settings.Set(ts_BoundedText, m_maxDistance > 10.0);

    m_newTask.Set(edit_text_task(Rect(m_p0, m_p1), utf8_string(""),
      m_settings));
    return TaskResult::CHANGE;
  }

  TaskResult MouseMove(const PosInfo& info) override{
    m_p1 = info.pos;
    info.status.SetMainText("Release to start editing text");
    info.status.SetText(str_from_to(m_p0, m_p1));
    m_maxDistance = std::max(m_maxDistance, distance(m_p0, m_p1));
    return TaskResult::DRAW;
  }

  TaskResult Preempt(const PosInfo&) override{
    m_newTask.Set(DefaultTask());
    return TaskResult::CHANGE;
  }
  MakeTextBox& operator=(const MakeTextBox&) = delete;

private:
  coord m_maxDistance;
  PendingTask m_newTask;
  Point m_p0;
  Point m_p1;
  Settings& m_settings;
};

Task* text_make_box_task(const Point& startPos, Settings& s)
{
  return new MakeTextBox(startPos, s);
}

} // namespace
