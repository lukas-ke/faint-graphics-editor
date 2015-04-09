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

#include <memory>
#include "util-wx/fwd-wx.hh"
#include "wx/sizer.h" // Fixme: Remove
#include "gui/tool-bar.hh"
#include "gui/tool-panel.hh"
#include "gui/tool-setting-panel.hh"

namespace faint{

ToolPanel::ToolPanel(wxWindow* parent,
  StatusInterface& status,
  ArtContainer& art,
  DialogContext& dialogContext,
  const StringSource& unitStrings)
{
  m_panel = create_panel(parent);

  const int borderSize = 5;
  wxBoxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);

  m_toolbar = std::make_unique<Toolbar>(m_panel, status, art);
  outerSizer->Add(m_toolbar->AsWindow(), 0,
    wxEXPAND|wxUP|wxLEFT|wxRIGHT, borderSize);

  m_toolSettingPanel = std::make_unique<ToolSettingPanel>(m_panel,
    status,
    art,
    dialogContext,
    unitStrings);
  outerSizer->Add(m_toolSettingPanel->AsWindow(),
    1, wxEXPAND | wxLEFT|wxRIGHT, borderSize);

  set_sizer(m_panel, outerSizer);
}

ToolPanel::~ToolPanel(){
  m_panel = nullptr; // Note: Deleted by wxWidgets
}

wxWindow* ToolPanel::AsWindow(){
  return m_panel;
}

bool ToolPanel::Visible() const{
  return m_panel->IsShown();
}

void ToolPanel::Show(bool show){
  m_panel->Show(show);
}

void ToolPanel::Enable(bool e){
  m_panel->Enable(e);
}

void ToolPanel::Hide(){
  Show(false);
}

void ToolPanel::ShowSettings(const Settings& s){
  m_toolSettingPanel->ShowSettings(s);
}

void ToolPanel::SelectTool(ToolId id){
  // Send an event like if the tool
  // Fixme: Weird, IIRC, used to put all handling in a FaintWindow
  // event-handler, and not duplicate button state...
  m_toolbar->SendToolChoiceEvent(id);
}

void ToolPanel::SelectLayer(Layer layer){
  // Fixme: See SelectTool note
  m_toolbar->SendLayerChoiceEvent(layer);
}

} // namespace
