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

#ifndef FAINT_TOOL_SETTING_PANEL_HH
#define FAINT_TOOL_SETTING_PANEL_HH
#include <list>

class wxWindow;

namespace faint{

class Art;
class DialogContext;
class Settings;
class StatusInterface;
class StringSource;
class ToolSettingCtrl;

class ToolSettingPanel{
public:
  ToolSettingPanel(wxWindow* parent,
    StatusInterface&,
    Art&,
    DialogContext&,
    const StringSource& units);

  wxWindow* AsWindow();
  void ShowSettings(const Settings&);
  ToolSettingPanel(const ToolSettingPanel&) = delete;
private:
  using ToolCtrlList = std::list<ToolSettingCtrl*>;
  ToolCtrlList m_toolControls;
  wxWindow* m_panel;
};

} // namespace

#endif
