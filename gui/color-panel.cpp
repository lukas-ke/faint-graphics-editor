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
#include "app/canvas.hh" // Fixme: Remove
#include "bitmap/bitmap.hh" // Fixme: Remove
#include "bitmap/color-span.hh" // Fixme: Remove
#include "util/setting-id.hh" // Fixme: Remove
#include "geo/int-size.hh"
#include "gui/color-panel.hh"
#include "gui/events.hh"
#include "gui/frame-ctrl.hh"
#include "gui/grid-ctrl.hh"
#include "gui/grid-dialog.hh"
#include "gui/paint-dialog.hh"
#include "gui/palette-ctrl.hh"
#include "gui/selected-color-ctrl.hh"
#include "gui/zoom-ctrl.hh"
#include "util-wx/bind-event.hh"
#include "util/dumb-ptr.hh"
#include "util/either.hh"

namespace faint{

static auto create_grid_ctrl(wxWindow* parent, AppContext& app,
  const Art& art,
  StatusInterface& status)
{
  auto showGridDialog =
    [&](){
    auto& canvas = app.GetActiveCanvas();

    auto result = show_grid_dialog(nullptr,
      canvas.GetGrid(),
      app.GetDialogContext());

    result.Visit([&](const Grid& grid){
        canvas.SetGrid(grid);
        canvas.Refresh();
      });
  };

  Accessor<Grid> gridAccess(
    [&](){
      return app.GetActiveCanvas().GetGrid();
    },
    [&](const Grid& grid){
      auto& canvas = app.GetActiveCanvas();
      canvas.SetGrid(grid);
      canvas.Refresh();
    });

  return make_dumb<GridCtrl>(parent, art, status, showGridDialog, gridAccess);
}

class ColorPanelImpl : public wxPanel {
public:
  ColorPanelImpl(wxWindow* parent,
    const PaintMap& palette,
    AppContext& app,
    StatusInterface& status,
    const Art& art)
    : wxPanel(parent)
  {
    // The spacing between controls in this panel
    const int spacing = 5;

    auto pickPaint =
      [&app, &art, &status](const utf8_string& title,
        const Paint& initial,
        const Color& secondary)
      {
        auto getBitmap = [&app](){
          const auto& background = app.GetActiveCanvas().GetBackground();
          return background.Visit(
            [&](const Bitmap& bmp) -> Bitmap{
              return bmp;
            },
            [&](const ColorSpan& colorSpan){
              // Not using the colorSpan size, since it might be huge, and
              // it would be pointless with only one color.
              const IntSize size(1,1);
              return Bitmap(size, colorSpan.color);
            });
        };

        return show_paint_dialog(nullptr, // Fixme: ?
          title,
          initial,
          secondary,
          getBitmap,
          art,
          status,
          app.GetDialogContext());
    };

    auto getBg = [&](){
      return get_color_default(app.Get(ts_Bg), color_white);
    };

    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    m_selectedColor = std::make_unique<SelectedColorCtrl>(this,
      IntSize(50,50),
      status,
      pickPaint,
      getBg);

    sizer->Add(m_selectedColor->AsWindow(), 0, wxALL, spacing);

    m_palette = std::make_unique<PaletteCtrl>(this,
      palette,
      status,
      pickPaint,
      getBg);
    sizer->Add(m_palette->AsWindow(), 0, wxALL | wxEXPAND | wxSHRINK, spacing);

    m_zoom = std::make_unique<ZoomCtrl>(this, status);
    sizer->Add(m_zoom->AsWindow(), 0, wxALL, spacing);

    m_grid = create_grid_ctrl(this, app, art, status);
    sizer->Add(m_grid.get(), 0, wxALL, spacing);

    m_frameCtrl = make_dumb<FrameCtrl>(this, app, status, art);
    sizer->Add(m_frameCtrl->AsWindow(), 0, wxALL, spacing);
    SetSizer(sizer);
    Layout();

    // Handle resizing child controls (e.g. color added to palette
    // or grid-control expanded).
    bind(this, EVT_FAINT_ControlResized,
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
  const PaintMap& palette,
  AppContext& app,
  StatusInterface& status,
  const Art& art)
  : m_impl(make_dumb<ColorPanelImpl>(parent, palette, app, status,
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
