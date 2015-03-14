// -*- coding: us-ascii-unix -*-
// Copyright 2015 Lukas Kemmer
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

#include "wx/bitmap.h"
#include "wx/dcclient.h" // For wxPaintDC
#include "wx/panel.h"
#include "bitmap/bitmap.hh"
#include "bitmap/draw.hh"
#include "geo/int-point.hh"
#include "geo/int-rect.hh"
#include "util/color-bitmap-util.hh"
#include "gui/dialog-context.hh"
#include "gui/paint-dialog/gradient-slider.hh"
#include "gui/paint-dialog/radial-gradient-display.hh"
#include "util-wx/bind-event.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/fwd-bind.hh"
#include "util-wx/fwd-wx.hh"

namespace faint{

class RadialGradientDisplay::RadialGradientDisplayImpl : public wxPanel {
public:
  RadialGradientDisplayImpl(wxWindow* parent,
    const wxSize& size,
    DialogContext& dialogContext)
    : wxPanel(parent, wxID_ANY)
  {
    SetBackgroundStyle(wxBG_STYLE_PAINT); // Prevent flicker on full refresh
    SetInitialSize(size);

    m_slider = std::make_unique<RadialGradientSlider>(this,
      IntSize(size.GetWidth(), g_sliderHeight),
      m_gradient,
      dialogContext);
    set_pos(m_slider->AsWindow(), {0, size.GetHeight() - g_sliderHeight});

    events::on_gradient_slider_change(*m_slider, [this](){
      UpdateBitmap();
      Refresh();
    });

    events::on_paint(this, [this](){
      wxPaintDC dc(this);
      dc.DrawBitmap(m_bmp, 0, 0);
    });
  }

  const RadialGradient& GetGradient() const{
    return m_gradient;
  }

  void SetGradient(const RadialGradient& g){
    m_gradient = g;
    UpdateBitmap();
    m_slider->UpdateGradient();
    Refresh();
  }

  void SetStops(const color_stops_t& stops){
    m_gradient.SetStops(stops);
    UpdateBitmap();
    m_slider->UpdateGradient();
    Refresh();
  }

  bool SetBackgroundColour(const wxColour& bgColor) override{
    wxPanel::SetBackgroundColour(bgColor);
    m_slider->SetBackgroundColor(to_faint(bgColor)); // Fixme: Warty
    UpdateBitmap();
    return true;
  }
private:
  void UpdateBitmap(){
    IntSize sz(to_faint(GetSize()));
    Bitmap bg(sz - IntSize(0, g_sliderHeight), to_faint(GetBackgroundColour()));

    IntSize gSz(sz - IntSize(10, g_sliderHeight));
    m_gradientBmp = subbitmap(gradient_bitmap(Gradient(m_gradient),
        IntSize(gSz.w * 2, gSz.h)),
      IntRect(IntPoint(gSz.w,0), gSz));
    blit(offsat(with_border(m_gradientBmp), {5,0}), onto(bg));
    m_bmp = to_wx_bmp(bg);
  }

  wxBitmap m_bmp;
  RadialGradient m_gradient;
  Bitmap m_gradientBmp;
  std::unique_ptr<RadialGradientSlider> m_slider;
};

RadialGradientDisplay::RadialGradientDisplay(wxWindow* parent,
  const IntSize& size,
  DialogContext& ctx)
{
  m_impl = new RadialGradientDisplayImpl(parent, to_wx(size), ctx);
}

RadialGradientDisplay::~RadialGradientDisplay(){
  m_impl = nullptr; // Deleted by wxWidgets
}

wxWindow* RadialGradientDisplay::AsWindow(){
  return m_impl;
}

const RadialGradient& RadialGradientDisplay::GetGradient() const{
  return m_impl->GetGradient();
}

void RadialGradientDisplay::Hide(){
  m_impl->Hide();
}

void RadialGradientDisplay::SetBackgroundColor(const Color& c){
  m_impl->SetBackgroundColour(to_wx(c));
}

void RadialGradientDisplay::SetStops(const color_stops_t& stops){
  m_impl->SetStops(stops);
}

void RadialGradientDisplay::SetGradient(const RadialGradient& g){
  m_impl->SetGradient(g);
}

void RadialGradientDisplay::Show(){
  m_impl->Show();
}

} // namespace
