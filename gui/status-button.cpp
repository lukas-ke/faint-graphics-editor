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

#include "gui/status-button.hh"
#include "util/status-interface.hh"
#include "util-wx/bind-event.hh"
#include "util-wx/convert-wx.hh"

namespace faint{

StatusButton::StatusButton(wxWindow* parent,
  const wxSize& size,
  StatusInterface& status,
  const utf8_string& label,
  const Tooltip& tooltip,
  const Description& description)
  : wxButton(parent, wxNewId(), to_wx(label), wxDefaultPosition,  size,
    // wxWANTS_CHARS prevents sound on keypress when button has focus
    wxWANTS_CHARS),
    m_description(description.Get()),
    m_status(status)
{
  SetToolTip(to_wx(tooltip.Get()));

  bind_fwd(this, wxEVT_LEAVE_WINDOW,
    [this](wxMouseEvent& event){
      m_status.SetMainText("");
      event.Skip();
    });

  bind_fwd(this, wxEVT_MOTION,
    [this](wxMouseEvent& event){
      // Set the status bar description. Done in EVT_MOTION-handler
      // because it did not work well in EVT_ENTER (the enter event
      // appears to be missed for example when moving between buttons)
      m_status.SetMainText(m_description);
      event.Skip();
    });
}

void StatusButton::UpdateText(const utf8_string& label,
  const Tooltip& tooltip,
  const Description& description)
{
  SetLabel(to_wx(label));
  m_description = description.Get();
  SetToolTip(to_wx(tooltip.Get()));
}

ToggleStatusButton::ToggleStatusButton(wxWindow* parent,
  int id,
  const wxSize& size,
  StatusInterface& status,
  const wxBitmap& bmpInactive,
  const wxBitmap& bmpActive,
  const Tooltip& tooltip,
  const Description& description)
: wxBitmapToggleButton(parent, id, bmpInactive,
  wxDefaultPosition,
  size, wxWANTS_CHARS|wxBORDER_NONE),
  m_description(description.Get()),
  m_status(status)
{
  SetBitmapFocus(bmpActive);
  SetBitmapCurrent(bmpActive);
  SetBitmapPressed(bmpActive);
  SetToolTip(to_wx(tooltip.Get()));

  bind_fwd(this, wxEVT_LEAVE_WINDOW,
    [this](wxMouseEvent& event){
      m_status.SetMainText("");
      event.Skip();
    });

  bind_fwd(this, wxEVT_MOTION,
    [this](wxMouseEvent& event){
      // Set the status bar description. Done in EVT_MOTION-handler
      // because it did not work well in EVT_ENTER (the enter event
      // appears to be missed for example when moving between buttons)
      m_status.SetMainText(m_description);
      event.Skip();
    });
}

} // namespace
