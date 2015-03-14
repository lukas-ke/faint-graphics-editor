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

#include "app/resource-id.hh"
#include "tasks/null-task.hh"
#include "tasks/task.hh"
#include "util/optional.hh"

namespace faint{

class NullTask : public Task{
public:
  void Activate() override{
  }

  TaskResult Char(const KeyInfo&) final override{
    return TaskResult::NONE;
  }

  TaskResult DoubleClick(const PosInfo&) override{
    return TaskResult::NONE;
  }

  void Draw(FaintDC&, Overlays&, const PosInfo&) override{
  }

  bool DrawBeforeZoom(Layer) const override{
    return false;
  }

  bool EatsSettings() const override{
    return false;
  }

  Command* GetCommand() override{
    return nullptr;
  }

  Cursor GetCursor(const PosInfo&) const override{
    return Cursor::DONT_CARE;
  }

  Task* GetNewTask() override{
    return nullptr;
  }

  IntRect GetRefreshRect(const RefreshInfo&) const override{
    return {};
  }

  Optional<const faint::HistoryContext&> HistoryContext() const override{
    return {};
  }

  TaskResult MouseDown(const PosInfo&) override{
    return TaskResult::NONE;
  }

  TaskResult MouseUp(const PosInfo&) override{
    return TaskResult::NONE;
  }

  TaskResult MouseMove(const PosInfo&) override{
    return TaskResult::NONE;
  }

  TaskResult Preempt(const PosInfo&) override{
    return TaskResult::NONE;
  }

  bool RefreshOnMouseOut() const override{
    return false;
  }

  void SelectionChange() override{
  }

  Optional<const faint::SelectionContext&> SelectionContext() const override{
    return {};
  }

  void SetLayer(Layer) override{
  }

  void UpdateSettings() override{
  }

  Optional<const faint::TextContext&> TextContext() const override{
    return {};
  }
};

Task* null_task(){
  return new NullTask();
}

} // namespace
