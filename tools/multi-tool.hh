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

#ifndef FAINT_MULTI_TOOL_HH
#define FAINT_MULTI_TOOL_HH
#include "commands/command.hh"
#include "tools/tool.hh"
#include "tasks/task.hh"
#include "util/settings.hh"

namespace faint{

class MultiTool : public Tool {
  // A base for tools which forward most things to tasks.
public:
  MultiTool(ToolId, default_task, initial_task initial=initial_task(nullptr));
  ToolResult Char(const KeyInfo&) override;
  ToolResult DoubleClick(const PosInfo&) override;
  void Draw(FaintDC&, Overlays&, const PosInfo&) override;
  bool DrawBeforeZoom(Layer) const override;
  bool EatsSettings() const override;
  Command* GetCommand() override;
  Cursor GetCursor(const PosInfo&) const override;
  IntRect GetRefreshRect(const RefreshInfo&) const override;
  Optional<const faint::HistoryContext&> HistoryContext() const override;
  ToolResult MouseDown(const PosInfo&) override;
  ToolResult MouseUp(const PosInfo&) override;
  ToolResult MouseMove(const PosInfo&) override;
  ToolResult Preempt(const PosInfo&) override;
  bool RefreshOnMouseOut() const override;
  void SelectionChange() override;
  Optional<const faint::SelectionContext&> SelectionContext() const override;
  void SetLayer(Layer) override;
  Optional<const faint::TextContext&> TextContext() const override;
protected:
  void UpdateTaskSettings();
private:
  ToolResult HandleTaskResult(TaskResult);
  Settings m_settings;
  TaskWrapper m_task;
  PendingCommand m_command;
};

} // namespace

#endif
