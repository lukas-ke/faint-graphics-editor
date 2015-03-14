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
#include "wx/panel.h"
#include "wx/sizer.h"
#include "gui/tool-bar.hh"
#include "gui/tool-panel.hh"
#include "gui/tool-setting-panel.hh"

namespace faint{

class ToolPanelImpl : public wxPanel{
public:
  ToolPanelImpl(wxWindow* parent,
    StatusInterface& status,
    ArtContainer& art,
    DialogContext& dialogContext)
    : wxPanel(parent)
  {
    const int borderSize = 5;
    wxBoxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);

    m_toolbar = std::make_unique<Toolbar>(this, status, art);
    outerSizer->Add(m_toolbar->AsWindow(), 0,
      wxEXPAND|wxUP|wxLEFT|wxRIGHT, borderSize);

    m_toolSettingPanel = new ToolSettingPanel(this, status, art, dialogContext);
    outerSizer->Add(m_toolSettingPanel, 1, wxEXPAND | wxLEFT|wxRIGHT,
      borderSize);
    SetSizerAndFit(outerSizer);
  }

  std::unique_ptr<Toolbar> m_toolbar;
  ToolSettingPanel* m_toolSettingPanel;
};

ToolPanel::ToolPanel(wxWindow* parent,
  StatusInterface& status,
  ArtContainer& art,
  DialogContext& dialogContext)
{
  m_impl = new ToolPanelImpl(parent, status, art, dialogContext);
}

ToolPanel::~ToolPanel(){
  // Note: deletion is handled by wxWidgets.
  m_impl = nullptr;
}

wxWindow* ToolPanel::AsWindow(){
  return m_impl;
}

bool ToolPanel::Visible() const{
  return m_impl->IsShown();
}

void ToolPanel::Show(bool show){
  m_impl->Show(show);
}

void ToolPanel::Enable(bool e){
  m_impl->Enable(e);
}

void ToolPanel::Hide(){
  Show(false);
}

void ToolPanel::ShowSettings(const Settings& s){
  m_impl->m_toolSettingPanel->ShowSettings(s);
}

void ToolPanel::SelectTool(ToolId id){
  // Send an event like if the tool
  // Fixme: Weird, IIRC, used to put all handling in a MainFrame
  // On...-handler, and not duplicate button state...
  m_impl->m_toolbar->SendToolChoiceEvent(id);
}

void ToolPanel::SelectLayer(Layer layer){
  // Fixme: See SelectTool note
  m_impl->m_toolbar->SendLayerChoiceEvent(layer);
}

} // namespace
