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

#include "tools/multi-tool.hh"
#include "util/optional.hh"

namespace faint{

MultiTool::MultiTool(ToolId id,
  default_task defaultTask,
  initial_task initialTask)
  : Tool(id),
    m_task(defaultTask, initialTask)
{}

ToolResult MultiTool::Char(const KeyInfo& info){
  return HandleTaskResult(m_task->Char(info));
}

void MultiTool::Draw(FaintDC& dc, Overlays& overlays, const PosInfo& posInfo){
  m_task->Draw(dc, overlays, posInfo);
}

bool MultiTool::DrawBeforeZoom(Layer layer) const{
  return m_task->DrawBeforeZoom(layer);
}

bool MultiTool::EatsSettings() const{
  return m_task->EatsSettings();
}

Command* MultiTool::GetCommand(){
  return m_command.Take();
}

Cursor MultiTool::GetCursor(const PosInfo& info) const{
  return m_task->GetCursor(info);
}

IntRect MultiTool::GetRefreshRect(const RefreshInfo& info) const{
  return m_task->GetRefreshRect(info);
}

Optional<const faint::HistoryContext&> MultiTool::HistoryContext() const{
  return m_task->HistoryContext();
}

ToolResult MultiTool::DoubleClick(const PosInfo& info){
  return HandleTaskResult(m_task->DoubleClick(info));
}

ToolResult MultiTool::MouseDown(const PosInfo& info){
  return HandleTaskResult(m_task->MouseDown(info));
}

ToolResult MultiTool::MouseUp(const PosInfo& info){
  return HandleTaskResult(m_task->MouseUp(info));
}

ToolResult MultiTool::MouseMove(const PosInfo& info){
  return HandleTaskResult(m_task->MouseMove(info));
}

ToolResult MultiTool::Preempt(const PosInfo& info){
  return HandleTaskResult(m_task->Preempt(info));
}

bool MultiTool::RefreshOnMouseOut() const{
  return m_task->RefreshOnMouseOut();
}

void MultiTool::SelectionChange(){
  m_task->SelectionChange();
}

Optional<const faint::SelectionContext&> MultiTool::SelectionContext() const{
  return m_task->SelectionContext();
}

void MultiTool::SetLayer(Layer layer){
  m_task->SetLayer(layer);
}

Optional<const faint::TextContext&> MultiTool::TextContext() const{
  return m_task->TextContext();
}

void MultiTool::UpdateTaskSettings(){
  m_task->UpdateSettings();
}

ToolResult MultiTool::HandleTaskResult(TaskResult r){
  if (r == TaskResult::DRAW){
    return ToolResult::DRAW;
  }
  else if (r == TaskResult::SETTING_CHANGED){
    return ToolResult::SETTING_CHANGED;
  }
  else if (r == TaskResult::CHANGE){
    if (m_task.Switch()){
      return ToolResult::DRAW;
    }
    return ToolResult::CHANGE;
  }
  else if (r == TaskResult::COMMIT){
    m_command.Set(m_task->GetCommand());
    return ToolResult::COMMIT;
  }
  else if (r == TaskResult::COMMIT_AND_CHANGE){
    m_command.Set(m_task->GetCommand());
    if (m_task.Switch()){
      return ToolResult::COMMIT;
    }
    return ToolResult::CHANGE;
  }
  else if (r == TaskResult::PUSH){
    m_task.Push();
    return ToolResult::DRAW;
  }
  return ToolResult::NONE;
}

} // namespace
