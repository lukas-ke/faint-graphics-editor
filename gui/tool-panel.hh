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
#include "util/status-interface.hh"

class wxWindow;

namespace faint{

class DialogContext;
class ToolPanelImpl;

class ToolPanel {
// Vertical panel containing a layer choice control, tool selection
// buttons and controls for the settings of the active tool or
// selected objects.
public:
  ToolPanel(wxWindow* parent, StatusInterface&, ArtContainer&, DialogContext&);
  ~ToolPanel();
  wxWindow* AsWindow();
  bool Visible() const;
  void Enable(bool);
  void Show(bool);
  void Hide();
  void SelectTool(ToolId);
  void SelectLayer(Layer);
  void ShowSettings(const Settings&);

private:
  ToolPanel(const ToolPanel&);
  ToolPanelImpl* m_impl;
};

} // namespace

#endif
