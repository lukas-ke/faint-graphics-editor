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

#ifndef FAINT_TOOL_PANEL_HH
#define FAINT_TOOL_PANEL_HH
#include <memory>
#include "util/status-interface.hh"

class wxWindow;

namespace faint{

class DialogContext;
class StringSource;
class Toolbar;
class ToolSettingPanel;

class ToolPanel {
// Vertical panel containing a layer choice control, tool selection
// buttons and controls for the settings of the active tool or
// selected objects.
public:
  ToolPanel(wxWindow* parent,
    StatusInterface&,
    ArtContainer&,
    DialogContext&,
    const StringSource& unitStrings);
  ~ToolPanel();
  wxWindow* AsWindow();
  void Enable(bool);
  void Hide();
  void SelectLayer(Layer);
  void SelectTool(ToolId);
  void Show(bool);
  void ShowSettings(const Settings&);
  bool Visible() const;

  ToolPanel(const ToolPanel&) = delete;
private:
  wxWindow* m_panel;
  std::unique_ptr<Toolbar> m_toolbar;
  std::unique_ptr<ToolSettingPanel> m_toolSettingPanel;
};

} // namespace

#endif
