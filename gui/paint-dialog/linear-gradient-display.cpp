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
#include "wx/dcclient.h"
#include "wx/window.h" // For allowing Bind
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/draw.hh"
#include "geo/adjust.hh"
#include "geo/angle.hh"
#include "geo/geo-func.hh"
#include "geo/int-point.hh"
#include "geo/line.hh"
#include "gui/common-cursors.hh"
#include "gui/dialog-context.hh"
#include "gui/mouse-capture.hh"
#include "gui/paint-dialog/gradient-slider.hh"
#include "gui/paint-dialog/linear-gradient-display.hh"
#include "util-wx/bind-event.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/fwd-bind.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/make-event.hh"
#include "util/color-bitmap-util.hh"

namespace faint{

MAKE_FAINT_COMMAND_EVENT(GRADIENT_ANGLE_PICKED);

static Bitmap with_angle_indicator(const Bitmap& src, const Angle& angle){
  IntSize sz(src.GetSize());
  int dx = static_cast<int>(cos(-angle) * sz.w);
  int dy = static_cast<int>(sin(-angle) * sz.w);
  int cx = sz.w / 2;
  int cy = sz.h / 2;
  Bitmap dst(src);
  Color border(50,50,50);
  Color fill(255,255,255);

  IntPoint start(cx, cy);
  IntPoint end(cx + dx, cy + dy);
  draw_line(dst, {start, end}, {border, 3, LineStyle::SOLID, LineCap::BUTT});
  draw_line(dst, {start, end}, solid_1px(fill));
  put_pixel(dst, start, border);
  return dst;
}

static IntPoint constrain_gradient_angle(const IntSize& sz, const IntPoint& p){
  const IntPoint c = point_from_size(sz / 2);
  return floored(adjust_to_45(floated(c), floated(p)));
}

class LinearGradientDisplay::LinearGradientDisplayImpl{
public:
  LinearGradientDisplayImpl(wxWindow* parent,
    const IntSize& size,
    const pick_color_f& getColor,
    DialogContext& dialogContext)
    : m_panel(create_panel(parent)),
      m_mouse(m_panel),
      m_offset(LinearGradientSlider::HORIZONTAL_MARGIN, 0)
  {
    set_bgstyle_paint(m_panel);
    set_initial_size(m_panel, size);

    m_slider = std::make_unique<LinearGradientSlider>(m_panel,
      IntSize(size.w, LinearGradientSlider::HEIGHT),
      m_gradient,
      getColor,
      dialogContext);
    set_pos(m_slider->AsWindow(), {0, size.h - LinearGradientSlider::HEIGHT});
    dialogContext.GetCommonCursors().SetCrosshair(m_panel);

    bind_fwd(m_panel, wxEVT_LEFT_DOWN,
      [this](wxMouseEvent& event){
        int max_x = m_bmp.GetWidth();
        int max_y = m_bmp.GetHeight();
        wxPoint pos(event.GetPosition());
        if (pos.x > max_x || pos.y > max_y){
          return;
        }
        m_mouse.Capture();
        SetAngleFromPos(GetBitmapPos(event));
      });

    events::on_mouse_left_up(m_panel, releaser(m_mouse));

    bind_fwd(m_panel, wxEVT_MOTION,
      [this](wxMouseEvent& event){
        if (m_mouse.HasCapture()){
          SetAngleFromPos(GetBitmapPos(event));
        }
      });

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

  const LinearGradient& GetGradient() const{
    return m_gradient;
  }

  bool SetBackgroundColor(const Color& bgColor){
    set_background_color(m_panel, bgColor);
    m_slider->SetBackgroundColor(bgColor);
    UpdateBitmap();
    return true;
  }

  void SetGradient(const LinearGradient& g){
    m_gradient = g;
    UpdateBitmap();
    m_slider->UpdateGradient();
    refresh(m_panel);
  }

  void SetStops(const color_stops_t& stops){
    m_gradient.SetStops(stops);
    m_slider->UpdateGradient();
    UpdateBitmap();
    refresh(m_panel);
  }

private:
  IntPoint GetBitmapPos(const wxMouseEvent& event){
    const IntPoint pos(to_faint(event.GetPosition()) - m_offset);
    return event.ShiftDown() ?
      constrain_gradient_angle(m_gradientBmp.GetSize(), pos) :
      pos;
  }

  void SetAngleFromPos(const IntPoint& mousePos){
    IntPoint pos = mousePos - point_from_size(m_gradientBmp.GetSize() / 2);
    if (pos.x == 0 && pos.y == 0){
      return;
    }
    Angle angle = -atan2(static_cast<double>(pos.y), static_cast<double>(pos.x));

    m_gradient.SetAngle(angle);
    UpdateBitmap();
    refresh(m_panel);
    wxCommandEvent event(EVT_GRADIENT_ANGLE_PICKED);
    // event.SetEventObject(m_panel);
    process_event(m_panel, event);
  }

  void UpdateBitmap(){
    auto panelSize = get_size(m_panel);
    Bitmap bg(panelSize, get_background_color(m_panel));

    const auto gradientSize = panelSize -
      IntSize(m_offset.x * 2, LinearGradientSlider::HEIGHT);
    m_gradientBmp = gradient_bitmap(Gradient(unrotated(m_gradient)), gradientSize);
    blit(offsat(with_border(with_angle_indicator(m_gradientBmp,
      m_gradient.GetAngle())), m_offset), onto(bg));
    m_bmp = to_wx_bmp(bg);
  }

  wxBitmap m_bmp;
  LinearGradient m_gradient;
  Bitmap m_gradientBmp;
  wxWindow* m_panel;
  MouseCapture m_mouse;
  std::unique_ptr<LinearGradientSlider> m_slider;
  const IntPoint m_offset;
};

LinearGradientDisplay::LinearGradientDisplay(wxWindow* parent,
  const IntSize& size,
  const pick_color_f& getColor,
  DialogContext& ctx)
{
  m_impl = std::make_unique<LinearGradientDisplayImpl>(parent,
    size,
    getColor,
    ctx);
}

LinearGradientDisplay::~LinearGradientDisplay(){
}

wxWindow* LinearGradientDisplay::AsWindow(){
  return m_impl->AsWindow();
}

const LinearGradient& LinearGradientDisplay::GetGradient() const{
  return m_impl->GetGradient();
}

void LinearGradientDisplay::Hide(){
  hide(m_impl->AsWindow());
}

void LinearGradientDisplay::SetBackgroundColor(const Color& c){
  m_impl->SetBackgroundColor(c);
}

void LinearGradientDisplay::SetStops(const color_stops_t& stops){
  m_impl->SetStops(stops);
}

void LinearGradientDisplay::SetGradient(const LinearGradient& g){
  m_impl->SetGradient(g);
}

void LinearGradientDisplay::Show(){
  show(m_impl->AsWindow());
}

} // namespace

namespace faint::events{

void on_angle_picked(LinearGradientDisplay& d, const std::function<void()>& f){
  bind(d.AsWindow(), EVT_GRADIENT_ANGLE_PICKED, f);
}

} // namespace
