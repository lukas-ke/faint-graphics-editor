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

#include "wx/bitmap.h"
#include "wx/window.h" // Fixme: Required for binding with bind-event
#include "wx/colour.h"
#include "wx/dcclient.h" // wxPaintDC
#include "bitmap/bitmap.hh"
#include "bitmap/draw.hh"
#include "bitmap/gradient.hh"
#include "bitmap/pattern.hh"
#include "geo/int-rect.hh"
#include "geo/int-size.hh"
#include "gui/mouse-capture.hh"
#include "gui/paint-dialog.hh" // Fixme: Pass lambda instead for color-selection?
#include "gui/paint-dialog/gradient-slider.hh"
#include "gui/dialog-context.hh"
#include "util/color-bitmap-util.hh"
#include "util/generator-adapter.hh"
#include "util/index.hh"
#include "util/named-operator.hh"
#include "util-wx/bind-event.hh"
#include "util-wx/fwd-bind.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/make-event.hh"
#include "util-wx/system-colors.hh"
#include "util/iter.hh"

namespace faint {

template<typename T>
static Color get_stop_color(const Index& index, const T& g){
  return g.GetStop(index).GetColor();
}

class HandleHitInfo{
public:
  enum HitType{
    MISS,
    MOVE_HANDLE,
    CHANGE_HANDLE_COLOR,
    ADD_HANDLE
  };

  static HandleHitInfo Miss(){
    return HandleHitInfo(MISS, 0, 0.0, color_white);
  }

  static HandleHitInfo HitMoveRegion(int index, const ColorStop& s){
    return HandleHitInfo(MOVE_HANDLE, index, s.GetOffset(), s.GetColor());
  }

  static HandleHitInfo HitColorChange(int index, const ColorStop& s){
    return HandleHitInfo(CHANGE_HANDLE_COLOR, index, s.GetOffset(), s.GetColor());
  }

  static HandleHitInfo HitAddHandle(double offset, const Color& c){
    return HandleHitInfo(ADD_HANDLE, 0, offset, c);
  }

  HitType type;
  int index;
  double offset;
  Color color;
private:
  HandleHitInfo(HitType type,
    int index,
    double offset,
    const Color& color)
    : type(type),
      index(index),
      offset(offset),
      color(color)
  {}
};

static void draw_color_stop_triangle(Bitmap& bmp,
  int cx,
  int w,
  int h,
  const BorderSettings& outline,
  const Paint& fill)
{
  std::vector<IntPoint> pts = {
    {cx, 0},
    {cx + w, h / 2 - 1},
    {cx - w, h / 2 - 1}};

  fill_polygon(bmp, pts, fill);
  draw_polygon(bmp, pts, outline);
}

static void draw_plus(Bitmap& bmp, const IntRect& r){
  int xc = r.Left() + r.w / 2;
  int yc = r.Top() + r.h / 2;
  const int w = (r.w - 4) / 2;
  const int h = (r.h - 4) / 2;
  auto s = solid_1px(color_black);
  draw_hline(bmp, yc, xc <plus_minus> w, s);
  draw_vline(bmp, xc, yc <plus_minus> h, s);
}

static const int HANDLE_WIDTH = 5;

static IntRect get_add_rectangle(int cx, const IntSize& sz){
  const int y1 = std::min(2  * HANDLE_WIDTH, sz.h / 2);
  return {IntPoint(cx - HANDLE_WIDTH, 2), IntPoint(cx + HANDLE_WIDTH, y1 + 2)};
}

static void draw_add_rectangle(Bitmap& bmp, const IntRect& r){
  fill_rect_color(bmp, r, color_button_face());
  draw_raised_ui_border(bmp, r);
  draw_plus(bmp, r);
}

static int pos_from_offset(double offset, const IntSize& sz){
  return HANDLE_WIDTH + floored((sz.w - 11) * offset); // Fixme: 11?
}

class ColorStopsRegion{
public:
  ColorStopsRegion() = default;

  ColorStopsRegion(const color_stops_t& stops,
    const IntSize& sz,
    const Color& bgColor)
  {
    Bitmap bmp(sz, bgColor);
    const int w = 5;

    // Add the regions for creating new handles
    const auto sortedStops = sorted(stops);

    const int y1 = std::min(2 * w, sz.h / 2);
    for (size_t i = 1; i < sortedStops.size(); i++){
      const ColorStop& s = sortedStops[i];
      int cx = pos_from_offset(s.GetOffset(), sz);
      const ColorStop& prevStop(sortedStops[i-1]);
      int prevX = pos_from_offset(prevStop.GetOffset(), sz);

      auto addRect = get_add_rectangle(mid_value(prevX, cx), sz);
      draw_add_rectangle(bmp, addRect);

      const double offset = mid_value(prevStop.GetOffset(), s.GetOffset());

      m_regions.push_back(std::make_pair(addRect,
          HandleHitInfo::HitAddHandle(offset,
            mix(prevStop.GetColor(), s.GetColor()))));
    }

    if (sortedStops.size() == 1){
      // Add an add-button at the opposite end when there's just one
      // stop (they're normally added at the mid-point between stops).
      const auto& stop = sortedStops.back();
      const double offset = stop.GetOffset() > 0.5 ? 0.0 : 1.0;
      const int cx = pos_from_offset(offset, sz);
      IntRect addRect(IntPoint(cx - w, 2), IntPoint(cx + w, y1 + 2));
      draw_add_rectangle(bmp, addRect);

      m_regions.push_back(std::make_pair(addRect,
          HandleHitInfo::HitAddHandle(offset, stop.GetColor())));
    }

    // Add the regions for moving a handle or changing its color
    for (size_t i = 0; i != stops.size(); i++){
      const ColorStop& s(stops[i]);
      int cx = pos_from_offset(s.GetOffset(), sz);

      // Use a bitmap for the fill, so that the color stop handles
      // indicate alpha with a checkered background
      const auto outline = solid_1px(color_black);
      Paint fill(Pattern(color_bitmap(s.GetColor(), IntSize(6,6))));

      draw_color_stop_triangle(bmp, cx, w, sz.h, outline, fill);

      int ry0 = sz.h / 2 + 2;
      int ry1= std::min(sz.h, ry0 + 2 * w + 1);
      m_regions.push_back(std::make_pair(IntRect(IntPoint(cx-w,0),
        IntPoint(cx+w, sz.h / 2 - 1)),
        HandleHitInfo::HitMoveRegion(resigned(i), s)));

      IntRect colorRect(IntPoint(cx - w, ry0), IntPoint(cx + w, ry1));
      fill_rect(bmp, colorRect, fill);
      draw_rect(bmp, colorRect, outline);
      m_regions.emplace_back(colorRect,
        HandleHitInfo::HitColorChange(resigned(i), s));
    }
    m_bmp = to_wx_bmp(bmp);
  }

  const wxBitmap& GetBitmap(){
    return m_bmp;
  }

  HandleHitInfo HitTest(const IntPoint& pos){
    // Traverse the regions in reverse so that the move-handles have
    // higher priority than the add-buttons
    for (auto region : reversed(m_regions)){
      if (region.first.Contains(pos)){
        return region.second;
      }
    }
    return HandleHitInfo::Miss();
  }

private:
  std::vector<std::pair<IntRect, HandleHitInfo> > m_regions;
  wxBitmap m_bmp;
};

MAKE_FAINT_COMMAND_EVENT(GRADIENT_SLIDER_CHANGE);

class ColorStopSliderImpl {
public:
  virtual ~ColorStopSliderImpl() = default;
  virtual wxWindow* AsWindow() = 0;
  virtual void UpdateGradient() = 0;
};

template<typename T>
class TypedColorStopSlider : public ColorStopSliderImpl {
public:
  TypedColorStopSlider(wxWindow* parent,
    const IntSize& size,
    T& gradient,
    DialogContext& dialogContext)
    : m_panel(create_panel(parent)),
      m_dialogContext(dialogContext),
      m_gradient(gradient),
      m_handle(-1),
      m_mouse(m_panel)
  {
    set_initial_size(m_panel, size);
    set_bgstyle_paint(m_panel);

    bind_fwd(m_panel, wxEVT_LEFT_DOWN,
      [this](wxMouseEvent& event){
        IntPoint pos(to_faint(event.GetPosition()));
        HandleHitInfo hit = m_region.HitTest(pos);
        m_handle = hit.index;

        if (hit.type == HandleHitInfo::MISS){
          return;
        }

        if (hit.type == HandleHitInfo::ADD_HANDLE){
          m_gradient.Add(ColorStop(hit.color, hit.offset));
          m_handle = m_gradient.GetNumStops().Get() - 1;
          m_mouse.Capture();
          SetHandleFromPos(to_faint(event.GetPosition()));
          return;
        }

        if (!event.ControlDown()){
          if (hit.type == HandleHitInfo::CHANGE_HANDLE_COLOR){
            PickHandleColor(Index(m_handle));
            return;
          }
        }
        else {
          // Clone the handle
          m_gradient.Add(m_gradient.GetStop(Index(m_handle)));
          m_handle = m_gradient.GetNumStops().Get() - 1;
        }
        m_mouse.Capture();
        SetHandleFromPos(to_faint(event.GetPosition()));
      });

    events::on_mouse_left_up(m_panel, releaser(m_mouse));

    bind_fwd(m_panel, wxEVT_MOTION,
      [this](wxMouseEvent& event){
        if (m_mouse.HasCapture()){
          SetHandleFromPos(to_faint(event.GetPosition()));
        }
        else {
          HandleHitInfo hit = m_region.HitTest(to_faint(event.GetPosition()));
          m_handle = hit.index;
          if (hit.type == HandleHitInfo::MISS){
            set_cursor(m_panel, wxCURSOR_ARROW);
          }
          else{
            if (hit.type == HandleHitInfo::MOVE_HANDLE ||
              hit.type == HandleHitInfo::ADD_HANDLE ||
              event.ControlDown())
            {
              set_cursor(m_panel, wxCURSOR_SIZEWE);
            }
            else{
              set_cursor(m_panel, wxCURSOR_HAND);
            }
          }
        }
      });

    events::on_paint(m_panel, [this](){
      wxPaintDC dc(m_panel);
      dc.DrawBitmap(m_region.GetBitmap(), 0, 0);
    });

    events::on_mouse_right_down(m_panel,
      [this](const IntPoint& pos){
        HandleHitInfo hit = m_region.HitTest(pos);
        m_handle = hit.index;
        if (hit.type == HandleHitInfo::MISS){
          return;
        }
        if (hit.type == HandleHitInfo::ADD_HANDLE){
          return;
        }
        if (m_gradient.GetNumStops() > 1){
          RemoveHandle(Index(m_handle));
        }
      });

    UpdateGradient();
  }

  wxWindow* AsWindow() override{
    return m_panel;
  }

  void UpdateGradient() override{
    m_region = ColorStopsRegion(m_gradient.GetStops(),
      get_size(m_panel),
      get_background_color(m_panel));
  }

private:
  void PickHandleColor(Index handle){
    ColorStop stop(m_gradient.GetStop(handle));
    Optional<Color> c = show_color_only_dialog(m_panel,
      "Select Stop Color", stop.GetColor(),
      m_dialogContext);
    if (c.IsSet()){
      m_gradient.SetStop(handle, ColorStop(c.Get(), stop.GetOffset()));
      UpdateGradient();
      SendChangeEvent();
      refresh(m_panel);
    }
  }

  void RemoveHandle(Index handle){
    m_gradient.Remove(handle);
    UpdateGradient();
    SendChangeEvent();
    refresh(m_panel);
  }

  void SendChangeEvent(){
    wxCommandEvent changeEvent(EVT_GRADIENT_SLIDER_CHANGE, get_id(m_panel));
    process_event(m_panel, changeEvent);
  }

  void SetHandleFromPos(const IntPoint& mousePos){
    coord x = constrained(Min(0.0),
      mousePos.x / floated(get_size(m_panel).w),
      Max(1.0));
    ColorStop stop(get_stop_color(Index(m_handle), m_gradient), x);
    m_gradient.SetStop(Index(m_handle), stop);
    UpdateGradient();
    SendChangeEvent();
    refresh(m_panel);
  }

  wxWindow* m_panel;

  DialogContext& m_dialogContext;
  T& m_gradient;
  int m_handle;
  MouseCapture m_mouse;
  ColorStopsRegion m_region;
};

LinearGradientSlider::LinearGradientSlider(wxWindow* parent,
  const IntSize& size,
  LinearGradient& gradient,
  DialogContext& ctx)
{
  m_impl = new TypedColorStopSlider<LinearGradient>(parent,
    size,
    gradient,
    ctx);
}

LinearGradientSlider::~LinearGradientSlider(){
  delete m_impl;
}

wxWindow* LinearGradientSlider::AsWindow(){
  return m_impl->AsWindow();
}

void LinearGradientSlider::UpdateGradient(){
  m_impl->UpdateGradient();
}

void LinearGradientSlider::SetBackgroundColor(const Color& c){
  set_background_color(m_impl->AsWindow(), c);
}

RadialGradientSlider::RadialGradientSlider(wxWindow* parent,
  const IntSize& size,
  RadialGradient& gradient,
  DialogContext& ctx)
{
  m_impl = new TypedColorStopSlider<RadialGradient>(parent,
    size,
    gradient,
    ctx);
}

RadialGradientSlider::~RadialGradientSlider(){
  delete m_impl;
}

wxWindow* RadialGradientSlider::AsWindow(){
  return m_impl->AsWindow();
}

void RadialGradientSlider::SetBackgroundColor(const Color& c){
  set_background_color(m_impl->AsWindow(), c);

}

void RadialGradientSlider::UpdateGradient(){
  m_impl->UpdateGradient();
}

} // namespace

namespace faint{ namespace events{

void on_gradient_slider_change(LinearGradientSlider& s,
  const std::function<void()>& f)
{
  bind(s.AsWindow(), EVT_GRADIENT_SLIDER_CHANGE, f);
}

void on_gradient_slider_change(RadialGradientSlider& s,
  const std::function<void()>& f)
{
  bind(s.AsWindow(), EVT_GRADIENT_SLIDER_CHANGE, f);
}

}} // namespace
