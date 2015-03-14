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
#include "wx/spinbutt.h"
#include "app/canvas.hh"
#include "app/get-app-context.hh"
#include "gui/art-container.hh"
#include "gui/drag-value-ctrl.hh"
#include "gui/events.hh"
#include "gui/grid-ctrl.hh"
#include "gui/spin-button.hh"
#include "util/grid.hh"
#include "util-wx/gui-util.hh"
#include "util-wx/fwd-bind.hh"
#include "util-wx/bind-event.hh"

namespace faint{

#ifdef __WXMSW__
#define GRIDCONTROL_BORDER_STYLE wxBORDER_THEME
#else
#define GRIDCONTROL_BORDER_STYLE wxBORDER_NONE
#endif

void update_grid_toggle_button(const Grid&, wxButton*, const ArtContainer&);

Grid get_active_grid(int delta=0){
  Grid g(get_app_context().GetActiveCanvas().GetGrid());
  g.SetSpacing(std::max(g.Spacing() + delta, 1));
  return g;
}

void set_active_grid(const Grid& g){
  Canvas& canvas = get_app_context().GetActiveCanvas();
  canvas.SetGrid(g);
  canvas.Refresh();
}

static void set_active_grid_spacing(int spacing){
  Grid g(get_active_grid());
  g.SetSpacing(spacing);
  set_active_grid(g);
}

static std::unique_ptr<SpinButton> grid_spinbutton(wxWindow* parent,
  wxSizer* sizer,
  winvec_t& showhide)
{
  auto spinButton = std::make_unique<SpinButton>(parent,
    IntSize(40,50),
    "Adjust Grid Spacing");
  spinButton->GetRaw()->Hide();
  sizer->Add(spinButton->GetRaw());
  showhide.push_back(spinButton->GetRaw());
  return spinButton;
}

DragValueCtrl* grid_text(wxWindow* parent,
  wxSizer* sizer,
  winvec_t& showhide,
  StatusInterface& statusInfo)
{
  DragValueCtrl* text = new DragValueCtrl(parent,
    IntRange(min_t(1)),
    Description("Drag to adjust grid spacing. Right-Click to disable grid."),
    statusInfo);
  showhide.push_back(text);
  text->Hide();
  sizer->Add(text, 0, wxALIGN_CENTER_VERTICAL);
  return text;
}

wxButton* grid_toggle_button(wxWindow* parent,
  wxSizer* sizer,
  const ArtContainer& art)
{
  wxButton* button = noiseless_button(parent, "", Tooltip(""),
    wxSize(60,50));
  update_grid_toggle_button(false, button, art);
  sizer->Add(button, 0, wxEXPAND);
  return button;
}

void update_grid_toggle_button(const Grid& g,
  wxButton* button,
  const ArtContainer& art)
{
  button->SetBitmap(art.Get(g.Enabled() ? Icon::GRID_OFF : Icon::GRID_ON));
  button->SetToolTip(g.Enabled() ? "Disable Grid" : "Enable Grid");
}

GridCtrl::GridCtrl(wxWindow* parent,
  const ArtContainer& art,
  StatusInterface& statusInfo)
  : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
    wxTAB_TRAVERSAL|GRIDCONTROL_BORDER_STYLE),
    m_art(art),
    m_enabled(false)
{
  m_sizer = new wxBoxSizer(wxHORIZONTAL);
  m_txtCurrentSize = grid_text(this, m_sizer, m_showhide, statusInfo);
  m_spinButton = grid_spinbutton(this, m_sizer, m_showhide);
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
      set_active_grid_spacing(m_newValue.Take());
    }
  });

  bind(this, wxEVT_SPIN_UP,
   [this](){
     m_newValue.Set(get_active_grid(+1).Spacing());
   });

  bind(this, wxEVT_SPIN_DOWN,
    [this](){
      m_newValue.Set(get_active_grid(-1).Spacing());
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

  Grid g(get_active_grid());
  g.SetEnabled(m_enabled);
  m_txtCurrentSize->SetValue(g.Spacing());
  update_grid_toggle_button(g, m_btnToggle, m_art);
  set_active_grid(g);
  SetSizerAndFit(m_sizer);
  send_control_resized_event(this);
}

void GridCtrl::Update(){
  Grid g(get_active_grid());
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
