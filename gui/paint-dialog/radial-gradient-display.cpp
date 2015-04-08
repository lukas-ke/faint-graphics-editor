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
#include "bitmap/bitmap.hh"
#include "bitmap/draw.hh"
#include "geo/int-point.hh"
#include "geo/int-rect.hh"
#include "gui/dialog-context.hh"
#include "gui/paint-dialog/gradient-slider.hh"
#include "gui/paint-dialog/radial-gradient-display.hh"
#include "util-wx/bind-event.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/fwd-bind.hh"
#include "util-wx/fwd-wx.hh"
#include "util/color-bitmap-util.hh"

namespace faint{

class RadialGradientDisplay::RadialGradientDisplayImpl{
public:
  RadialGradientDisplayImpl(wxWindow* parent,
    const IntSize& size,
    DialogContext& dialogContext)
  {
    m_panel = create_panel(parent);
    set_bgstyle_paint(m_panel);
    set_initial_size(m_panel, size);

    m_slider = std::make_unique<RadialGradientSlider>(m_panel,
      IntSize(size.w, RadialGradientSlider::HEIGHT),
      m_gradient,
      dialogContext);
    set_pos(m_slider->AsWindow(), {0, size.h - RadialGradientSlider::HEIGHT});

    events::on_gradient_slider_change(*m_slider, [this](){
      UpdateBitmap();
      refresh(m_panel);
    });

    events::on_paint(m_panel, [this](){
      wxPaintDC dc(m_panel);
      dc.DrawBitmap(m_bmp, 0, 0);
    });
  }

  wxWindow* AsWindow(){
    return m_panel;
  }

  const RadialGradient& GetGradient() const{
    return m_gradient;
  }

  bool SetBackgroundColor(const Color& bgColor){
    set_background_color(m_panel, bgColor);
    m_slider->SetBackgroundColor(bgColor);
    UpdateBitmap();
    return true;
  }

  void SetGradient(const RadialGradient& g){
    m_gradient = g;
    UpdateBitmap();
    m_slider->UpdateGradient();
    refresh(m_panel);
  }

  void SetStops(const color_stops_t& stops){
    m_gradient.SetStops(stops);
    UpdateBitmap();
    m_slider->UpdateGradient();
    refresh(m_panel);
  }

private:
  void UpdateBitmap(){
    auto panelSize = get_size(m_panel);
    Bitmap bg(panelSize - IntSize(0, RadialGradientSlider::HEIGHT),
      get_background_color(m_panel));

    auto xOffset = RadialGradientSlider::HORIZONTAL_MARGIN;
    auto gradientSize(panelSize -
      IntSize(2 * xOffset, RadialGradientSlider::HEIGHT));

    // Use the right-half of the gradient ellipse
    m_gradientBmp = subbitmap(gradient_bitmap(Gradient(m_gradient),
        IntSize(gradientSize.w * 2, gradientSize.h)),
      IntRect(IntPoint(gradientSize.w, 0), gradientSize));
    blit(offsat(with_border(m_gradientBmp), {xOffset, 0}), onto(bg));
    m_bmp = to_wx_bmp(bg);
  }

  wxBitmap m_bmp;
  RadialGradient m_gradient;
  Bitmap m_gradientBmp;
  wxWindow* m_panel;
  std::unique_ptr<RadialGradientSlider> m_slider;
};

RadialGradientDisplay::RadialGradientDisplay(wxWindow* parent,
  const IntSize& size,
  DialogContext& ctx)
{
  m_impl = std::make_unique<RadialGradientDisplayImpl>(parent, size, ctx);
}

RadialGradientDisplay::~RadialGradientDisplay(){
}

wxWindow* RadialGradientDisplay::AsWindow(){
  return m_impl->AsWindow();
}

const RadialGradient& RadialGradientDisplay::GetGradient() const{
  return m_impl->GetGradient();
}

void RadialGradientDisplay::Hide(){
  hide(m_impl->AsWindow());
}

void RadialGradientDisplay::SetBackgroundColor(const Color& c){
  m_impl->SetBackgroundColor(c);
}

void RadialGradientDisplay::SetStops(const color_stops_t& stops){
  m_impl->SetStops(stops);
}

void RadialGradientDisplay::SetGradient(const RadialGradient& g){
  m_impl->SetGradient(g);
}

void RadialGradientDisplay::Show(){
  show(m_impl->AsWindow());
}

} // namespace
