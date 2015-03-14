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

#ifndef FAINT_STATUS_BUTTON_HH
#define FAINT_STATUS_BUTTON_HH
#include "wx/button.h"
#include "wx/tglbtn.h"
#include "gui/gui-string-types.hh"

namespace faint{

class StatusInterface;

class StatusButton : public wxButton{
  // Button which uses a StatusInterface to display description text
public:
  StatusButton(wxWindow* parent,
    const wxSize&,
    StatusInterface&,
    const utf8_string& label,
    const Tooltip&,
    const Description&);

  void UpdateText(const utf8_string& label,
    const Tooltip&,
    const Description&);

private:
  utf8_string m_description;
  StatusInterface& m_status;
};

class ToggleStatusButton : public wxBitmapToggleButton {
  // An enable/disable bitmap button which uses a StatusInterface to
  // display description text
public:
  ToggleStatusButton(wxWindow* parent,
    int id,
    const wxSize&,
    StatusInterface&,
    const wxBitmap& bmpInactive,
    const wxBitmap& bmpActive,
    const Tooltip&,
    const Description&);

private:
  utf8_string m_description;
  StatusInterface& m_status;
};

} // namespace

#endif
