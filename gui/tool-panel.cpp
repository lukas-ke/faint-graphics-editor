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

#include "wx/defs.h" // wxEXPAND etc
#include "gui/tool-bar.hh"
#include "gui/tool-panel.hh"
#include "gui/tool-setting-panel.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/layout-wx.hh"

namespace faint{

ToolPanel::ToolPanel(wxWindow* parent,
  StatusInterface& status,
  Art& art,
  DialogContext& dialogContext,
  const StringSource& unitStrings)
{
  m_panel = create_panel(parent);
  m_toolbar = std::make_unique<Toolbar>(m_panel, status, art);
  m_toolSettingPanel = std::make_unique<ToolSettingPanel>(m_panel,
    status,
    art,
    dialogContext,
    unitStrings);

  const int borderSize = from_DIP(5, m_panel);

  using namespace layout;
  set_sizer(m_panel,
    create_column(OuterSpacing(0), ItemSpacing(0), {
      {m_toolbar->AsWindow(),
       Proportion(0),
       wxEXPAND|wxUP|wxLEFT|wxRIGHT, borderSize},

      {m_toolSettingPanel->AsWindow(),
       Proportion(1),
       wxEXPAND|wxLEFT|wxRIGHT, borderSize}}));
}

ToolPanel::~ToolPanel(){
  deleted_by_wx(m_panel);
}

wxWindow* ToolPanel::AsWindow(){
  return m_panel;
}

bool ToolPanel::Visible() const{
  return is_shown(m_panel);
}

void ToolPanel::Show(bool s){
  show(m_panel, s);
}

void ToolPanel::Enable(bool e){
  enable(m_panel, e);
}

void ToolPanel::Hide(){
  hide(m_panel);
}

void ToolPanel::ShowSettings(const Settings& s){
  m_toolSettingPanel->ShowSettings(s);
}

void ToolPanel::SelectTool(ToolId id){
  // Fixme: Weird, IIRC, used to put all handling in a FaintWindow
  // event-handler, and not duplicate button state...
  m_toolbar->SendToolChoiceEvent(id);
}

void ToolPanel::SelectLayer(Layer layer){
  // Fixme: See SelectTool note
  m_toolbar->SendLayerChoiceEvent(layer);
}

} // namespace
