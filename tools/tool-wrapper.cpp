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

#include "app/app-context.hh"
#include "tools/tool.hh"
#include "tools/tool-wrapper.hh"

namespace faint{

ToolWrapper::ToolWrapper(AppContext& app)
  : m_app(app),
    m_lastSeen(nullptr),
    m_switched(nullptr)
{}

bool ToolWrapper::DrawBeforeZoom() const{
  return GetTool().DrawBeforeZoom(m_app.GetLayerType());
}

Tool& ToolWrapper::GetTool(){
  if (m_switched == nullptr){
    m_lastSeen = m_app.GetActiveTool();
    assert(m_lastSeen != nullptr);
    m_lastSeen->SetLayer(m_app.GetLayerType());
    return *m_lastSeen;
  }
  else {
    Tool* mainTool = m_app.GetActiveTool();
    if (m_lastSeen != mainTool){

      // The selected tool has been switched by the user since a tool
      // requested a switch. Lose the switch-info and revert to
      // using the common tool
      m_switched = nullptr;
      m_lastSeen = mainTool;
      assert(mainTool != nullptr);
      mainTool->SetLayer(m_app.GetLayerType());
      return *mainTool;
    }
    else {
      return *m_switched;
    }
  }
}

const Tool& ToolWrapper::GetTool() const{
  Tool& tool(const_cast<ToolWrapper*>(this)->GetTool());
  tool.SetLayer(m_app.GetLayerType());
  return tool;
}

ToolId ToolWrapper::GetToolId() const{
  return GetTool().GetId();
}

void ToolWrapper::SetSwitched(Tool* tool){
  Tool* mainTool = m_app.GetActiveTool();
  if (mainTool == tool){
    // Don't use the commonly selected tool as the switched tool,
    // just clear the switch info
    m_lastSeen = mainTool;
    m_switched = nullptr;
  }
  else {
    m_lastSeen = mainTool;
    m_switched = tool;
  }
}

void ToolWrapper::ClearSwitched(){
  delete m_switched;
  m_switched = nullptr;
  m_lastSeen = m_app.GetActiveTool();
}

} // namespace
