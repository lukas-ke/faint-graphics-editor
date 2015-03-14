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

#include "tools/standard-tool.hh"
#include "util/setting-util.hh"
#include "util/pos-info.hh"

namespace faint{

bool sub_pixel(const PosInfo& info){
  return is_object(info.layerType);
}

StandardTool::StandardTool(ToolId toolId, const Settings& s)
  : Tool(toolId),
    m_settings(s)
{}

ToolResult StandardTool::Char(const KeyInfo&){
  return ToolResult::NONE;
}

ToolResult StandardTool::DoubleClick(const PosInfo& info){
  // Default to forwarding the double click as another left down
  return MouseDown(info);
}

bool StandardTool::EatsSettings() const{
  return false;
}

const Settings& StandardTool::GetSettings() const{
  return m_settings;
}

bool StandardTool::SetAntiAlias(const PosInfo& info){
  bool subPixel = sub_pixel(info);
  m_settings.Set(ts_AntiAlias, subPixel);
  return subPixel;
}

bool StandardTool::GetAntiAlias() const{
  return m_settings.Get(ts_AntiAlias);
}

Optional<const faint::HistoryContext&> StandardTool::HistoryContext() const{
  return {};
}

bool StandardTool::RefreshOnMouseOut() const{
  return false;
}

void StandardTool::SelectionChange(){
}

void StandardTool::SetLayer(Layer){
}

void StandardTool::SetSwapColors(bool swap){
  m_settings.Set(ts_SwapColors, swap);
}

bool StandardTool::Set(const BoundSetting& s){
  return m_settings.Update(s);
}

Optional<const faint::SelectionContext&> StandardTool::SelectionContext() const{
  return {};
}

bool StandardTool::UpdateSettings(const Settings& s){
  return m_settings.Update(s);
}

Optional<const faint::TextContext&> StandardTool::TextContext() const{
  return {};
}

} // namespace
