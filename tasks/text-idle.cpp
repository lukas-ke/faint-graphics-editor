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

#include "tasks/standard-task.hh"
#include "tasks/text-edit.hh"
#include "tasks/text-idle.hh"
#include "tasks/text-make-box.hh"
#include "text/formatting.hh"
#include "util/pos-info.hh"
#include "util/tool-util.hh"

namespace faint{

class TextIdle : public StandardTask {
public:
  TextIdle(Settings& s)
    : m_settings(s)
  {}

  void Draw(FaintDC&, Overlays&, const PosInfo&) override{
  }

  bool DrawBeforeZoom(Layer layer) const override{
    return layer == Layer::RASTER;
  }

  Command* GetCommand() override{
    return nullptr;
  }

  Cursor GetCursor(const PosInfo& info) const override{
    return (hovered_selected_text(info) == nullptr) ?
      Cursor::TEXT_CROSS :
      Cursor::CARET;
  }

  IntRect GetRefreshRect(const RefreshInfo&) const override{
    return {};
  }

  Task* GetNewTask() override{
    return m_newTask.Take();
  }

  TaskResult MouseDown(const PosInfo& info) override{
    ObjText* objText = hovered_selected_text(info);
    if (objText == nullptr){
      m_newTask.Set(text_make_box_task(info.pos, m_settings));
    }
    else {
      m_newTask.Set(edit_text_task(objText, m_settings));
    }
    return TaskResult::CHANGE;
  }

  TaskResult MouseUp(const PosInfo&) override{
    return TaskResult::NONE;
  }

  TaskResult MouseMove(const PosInfo& info) override{
    if (hovered_selected_text(info)){
      info.status.SetMainText("Click to edit the text object");
    }
    else {
      info.status.SetMainText("Click to start drawing a text-box");
    }
    info.status.SetText(str(info.pos));
    return TaskResult::NONE;
  }

  TaskResult Preempt(const PosInfo&) override{
    return TaskResult::NONE;
  }

  TextIdle& operator=(const TextIdle&) = delete;
private:
  PendingTask m_newTask;
  Settings& m_settings;

};

Task* text_idle_task(Settings& s){
  return new TextIdle(s);
}

} // namespace
