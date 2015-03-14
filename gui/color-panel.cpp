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

#include "wx/panel.h"
#include "wx/sizer.h"
#include "app/app-context.hh" // Fixme: Remove
#include "geo/int-size.hh"
#include "gui/color-panel.hh"
#include "gui/events.hh"
#include "gui/frame-ctrl.hh"
#include "gui/grid-ctrl.hh"
#include "gui/palette-ctrl.hh"
#include "gui/selected-color-ctrl.hh"
#include "gui/zoom-ctrl.hh"
#include "util/dumb-ptr.hh"
#include "util-wx/bind-event.hh"

namespace faint{

class ColorPanelImpl : public wxPanel {
public:
  ColorPanelImpl(wxWindow* parent,
    const Settings& settings,
    const PaintMap& palette,
    AppContext& app,
    StatusInterface& status,
    const ArtContainer& art)
    : wxPanel(parent)
  {
    // The spacing between controls in this panel
    const int spacing = 5;

    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    m_selectedColor = std::make_unique<SelectedColorCtrl>(this, IntSize(50,50),
      status, app.GetDialogContext());
    sizer->Add(m_selectedColor->AsWindow(), 0, wxALL, spacing);

    m_palette = std::make_unique<PaletteCtrl>(this,
      settings,
      palette,
      status,
      app.GetDialogContext());
    sizer->Add(m_palette->AsWindow(), 0, wxALL | wxEXPAND | wxSHRINK, spacing);

    m_zoom = std::make_unique<ZoomCtrl>(this, status);
    sizer->Add(m_zoom->AsWindow(), 0, wxALL, spacing);

    m_grid = make_dumb<GridCtrl>(this, art, status);
    sizer->Add(m_grid.get(), 0, wxALL, spacing);

    m_frameCtrl = make_dumb<FrameCtrl>(this, app, status, art);
    sizer->Add(m_frameCtrl->AsWindow(), 0, wxALL, spacing);
    SetSizer(sizer);
    Layout();

    // Handle resizing child controls (e.g. color added to palette
    // or grid-control expanded).
    bind(this, EVT_FAINT_CONTROL_RESIZED, 
      [&](){
        Layout();
        Refresh();
      });
  }

  std::unique_ptr<ZoomCtrl> m_zoom;
  dumb_ptr<GridCtrl> m_grid;
  std::unique_ptr<SelectedColorCtrl> m_selectedColor;
  std::unique_ptr<PaletteCtrl> m_palette;
  dumb_ptr<FrameCtrl> m_frameCtrl;
};

ColorPanel::ColorPanel(wxWindow* parent,
  const Settings& settings,
  const PaintMap& palette,
  AppContext& app,
  StatusInterface& status,
  const ArtContainer& art)
  : m_impl(make_dumb<ColorPanelImpl>(parent, settings, palette, app, status,
      art))
{}

void ColorPanel::AddToPalette(const Paint& paint){
  m_impl->m_palette->Add(paint);
}

wxWindow* ColorPanel::AsWindow(){
  return m_impl.get();
}

void ColorPanel::Freeze(){
  m_impl->Freeze();
}

void ColorPanel::Hide(){
  Show(false);
}

void ColorPanel::SetPalette(const PaintMap& paintMap){
  m_impl->m_palette->SetPalette(paintMap);
}

void ColorPanel::Show(bool show){
  m_impl->Show(show);
}

void ColorPanel::Thaw(){
  m_impl->Thaw();
}

void ColorPanel::UpdateFrames(){
  if (m_impl->m_frameCtrl->Update()){
    m_impl->Layout();
  }
}

void ColorPanel::UpdateGrid(){
  m_impl->m_grid->Update();
}

void ColorPanel::UpdateSelectedColors(const ColorChoice& c){
  m_impl->m_selectedColor->UpdateColors(c);
}

void ColorPanel::UpdateZoom(const ZoomLevel& zoom){
  m_impl->m_zoom->UpdateZoom(zoom);
}

bool ColorPanel::Visible() const{
  return m_impl->IsShown();
}

} // namespace
