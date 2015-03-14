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

#ifndef FAINT_TOOL_DROP_DOWN_BUTTON_HH
#define FAINT_TOOL_DROP_DOWN_BUTTON_HH
#include <vector>
#include "wx/bitmap.h"
#include "wx/tglbtn.h"
#include "gui/gui-string-types.hh"
#include "tools/tool-id.hh"

class wxWindow;

namespace faint{

class IntSize;

struct ToolInfo{
  ToolInfo(const wxBitmap& inactive,
    const wxBitmap& active,
    const Tooltip&,
    const Description&,
    const ToolId&);
  wxBitmap inactive;
  wxBitmap active;
  Tooltip tooltip;
  Description description;
  ToolId toolId;
};

class ToolDropDownButton : public wxBitmapToggleButton{
public:
  ToolDropDownButton(wxWindow*,
    const std::vector<ToolInfo>&,
    const IntSize&);

  bool SetSelectedTool(ToolId);
  void RefreshHover();
private:
  std::vector<ToolInfo> m_items;
};

ToolDropDownButton* tool_drop_down_button(wxWindow*,
  const IntSize&,
  const std::vector<ToolInfo>&);

} // namespace

#endif
