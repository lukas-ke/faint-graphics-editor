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

#include <algorithm>
#include "wx/button.h"
#include "wx/sizer.h"
#include "app/resource-id.hh"
#include "gui/art.hh"
#include "gui/drag-value-ctrl.hh"
#include "gui/events.hh"
#include "gui/grid-ctrl.hh"
#include "gui/spin-button.hh"
#include "util-wx/bind-event.hh"
#include "util-wx/fwd-bind.hh"
#include "util-wx/fwd-wx.hh"
#include "util/grid.hh"

namespace faint{

// Border doesn't look nice on GTK, show it only on windows
#ifdef __WXMSW__
const auto GRIDCONTROL_BORDER_STYLE = wxBORDER_THEME;
#else
const auto GRIDCONTROL_BORDER_STYLE = wxBORDER_NONE;
#endif

static void update_grid_toggle_button(const Grid& g,
  wxButton* button,
  const Art& art)
{
  button->SetBitmap(art.Get(g.Enabled() ? Icon::GRID_OFF : Icon::GRID_ON));
  button->SetToolTip(g.Enabled() ? "Disable Grid" : "Enable Grid");
}

static Grid offset_spacing(Grid g, int delta=0){
  g.SetSpacing(std::max(g.Spacing() + delta, 1));
  return g;
}

static void set_active_grid_spacing(Accessor<Grid>& gridAccess, int spacing){
  Grid g = gridAccess.Get();
  g.SetSpacing(spacing);
  gridAccess.Set(g);
}

static std::unique_ptr<SpinButton> grid_spinbutton(wxWindow* parent,
  wxSizer* sizer,
  winvec_t& showhide,
  const on_spin_up& onSpinUp,
  const on_spin_down& onSpinDown)
{
  // Create the buttons for single-stepping the spacing

  auto spinButton = std::make_unique<SpinButton>(parent,
    IntSize(40,50),
    "Adjust Grid Spacing",
    onSpinUp,
    onSpinDown);
  spinButton->GetRaw()->Hide();
  sizer->Add(spinButton->GetRaw());
  showhide.push_back(spinButton->GetRaw());
  return spinButton;
}

DragValueCtrl* grid_text(wxWindow* parent,
  wxSizer* sizer,
  winvec_t& showhide,
  const Art& art,
  StatusInterface& statusInfo,
  const DialogFunc& showDialog)
{
  // Create the drag-adjustable grid spacing text

  DragValueCtrl* text = new DragValueCtrl(parent,
    IntRange(min_t(1)),
    Description(
     "Grid: Drag to adjust spacing. "
     "Right-Click to disable. "
     "Double-click for dialog"),
    DragCursor(art.Get(Cursor::DRAG_SCALE)),
    HoverCursor(art.Get(Cursor::MOVE_POINT)),
    statusInfo);
  showhide.push_back(text);
  text->Hide();

  events::on_mouse_left_double_click(text,
    [=](const IntPoint&){
      showDialog();
    });

  sizer->Add(text, 0, wxALIGN_CENTER_VERTICAL);
  return text;
}

wxButton* grid_toggle_button(wxWindow* parent,
  wxSizer* sizer,
  const Art& art)
{
  // Create the button that enables/disables the grid
  wxButton* button = noiseless_button(parent,
    "",
    Tooltip(""),
    IntSize(60,50));
  update_grid_toggle_button(false, button, art);
  sizer->Add(button, 0, wxEXPAND);
  return button;
}

GridCtrl::GridCtrl(wxWindow* parent,
  const Art& art,
  StatusInterface& statusInfo,
  const DialogFunc& showDialog,
  const Accessor<Grid>& grid)
  : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
    wxTAB_TRAVERSAL|GRIDCONTROL_BORDER_STYLE),
    m_art(art),
    m_enabled(false),
    m_grid(grid)
{
  m_sizer = new wxBoxSizer(wxHORIZONTAL);
  m_txtCurrentSize = grid_text(this, m_sizer, m_showhide, m_art, statusInfo,
    showDialog);
  m_spinButton = grid_spinbutton(this, m_sizer, m_showhide,
    on_spin_up([this](){
      auto g = offset_spacing(m_grid.Get(), +1);
      m_newValue.Set(g.Spacing());
    }),
    on_spin_down([this](){
      auto g = offset_spacing(m_grid.Get(), -1);
      m_newValue.Set(g.Spacing());
  }));

  m_btnToggle = grid_toggle_button(this, m_sizer, art);

  events::on_mouse_right_down(m_txtCurrentSize, [this](const IntPoint&){
    EnableGrid(false);
  });

  SetSizerAndFit(m_sizer);

  events::on_button(m_btnToggle, [this](){
    EnableGrid(!m_enabled);
  });

  events::on_idle(this, [this](){
    if (m_newValue.IsSet()){
      // Update the text in idle-handler, presumably to avoid refresh
      // glitches when dragging the value.
      set_active_grid_spacing(m_grid, m_newValue.Take());
    }
  });

  bind_fwd(this, EVT_FAINT_DRAG_VALUE_CHANGE,
    [this](wxCommandEvent& event){
      m_newValue.Set(event.GetInt());
    });
}

void GridCtrl::EnableGrid(bool enable){
  m_enabled = enable;

  // Show items if enabled, otherwise hide
  for (wxWindow* window : m_showhide){
    window->Show(m_enabled);
  }

  Grid g(m_grid.Get());
  g.SetEnabled(m_enabled);
  m_txtCurrentSize->SetValue(g.Spacing());
  update_grid_toggle_button(g, m_btnToggle, m_art);
  m_grid.Set(g);
  SetSizerAndFit(m_sizer);
  send_control_resized_event(this);
}

void GridCtrl::Update(){
  Grid g = m_grid.Get();
  m_txtCurrentSize->SetValue(g.Spacing());
  const bool enabled = g.Enabled();

  if (m_enabled != enabled){
    update_grid_toggle_button(g, m_btnToggle, m_art);
    m_enabled = enabled;
    for (wxWindow* window : m_showhide){
      window->Show(m_enabled);
    }
    SetSizerAndFit(m_sizer);
    send_control_resized_event(this);
  }
}

} // namespace
