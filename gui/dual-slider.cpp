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
#include "wx/bitmap.h"
#include "wx/dcclient.h"
#include "app/get-art-container.hh" // Fixme: Pass it (or cursors) instead
#include "app/resource-id.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/draw.hh"
#include "geo/line.hh"
#include "gui/art-container.hh"
#include "gui/dual-slider.hh"
#include "gui/mouse-capture.hh"
#include "util-wx/bind-event.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/fwd-bind.hh"
#include "util-wx/system-colors.hh"

namespace faint{

class DualSliderEvent : public wxCommandEvent{
public:
  DualSliderEvent(const Interval&, bool special);
  wxEvent* Clone() const;
  Interval GetSelectedInterval() const;
  bool Special() const;
private:
  Interval m_interval;
  bool m_special;
};

const wxEventType FAINT_DUAL_SLIDER_CHANGE = wxNewEventType();
const wxEventTypeTag<DualSliderEvent> EVT_FAINT_DUAL_SLIDER_CHANGE(
  FAINT_DUAL_SLIDER_CHANGE);


DualSlider::DualSlider(wxWindow* parent)
  : wxPanel(parent)
{}

class DualSliderImpl : public DualSlider{
public:
  DualSliderImpl(wxWindow* parent,
    const ClosedIntRange& range,
    const Interval& startInterval,
    const SliderBackground& bg)
    : DualSlider(parent),
      m_anchor(0),
      m_anchorV1(0.0),
      m_anchorV2(0.0),
      m_background(bg.Clone()),
      m_mouse(this),
      m_range(range),
      m_v1(startInterval.Lower()),
      m_v2(startInterval.Upper()),
      m_which(0)
  {
    assert(m_v1 < m_v2);
    assert(m_range.Has(m_v1));
    assert(m_range.Has(m_v2));

    SetInitialSize(wxSize(20,20)); // Minimum size

    events::on_mouse_left_down(this,
      [this](const IntPoint& pos){
        MouseDown(pos, false);
      });

    events::on_mouse_right_down(this,
      [this](const IntPoint& pos){
        MouseDown(pos, true);
      });

    auto handle_mouse_up = [this](const IntPoint&){
      if (m_mouse.HasCapture()){
        m_mouse.Release();
        Refresh();
      }
    };

    events::on_mouse_left_up(this, handle_mouse_up);
    events::on_mouse_right_up(this, handle_mouse_up);

    events::on_mouse_motion(this,
      [this](const IntPoint& pos){
        if (m_mouse.HasCapture()){
          int x = pos.x;
          int w = GetSize().GetWidth();
          if (m_which != 2){
            UpdateFromPos(m_range.Constrain(pos_to_value(x, w, m_range)),
              m_which);
          }
          else{
            UpdateFromAnchor(x);
          }
          Refresh();
          SendSliderChangeEvent();
        }
        else{
          int handle = WhichHandle(pos.x);
          if (handle <= 1){
            SetCursor(get_art_container().Get(Cursor::HORIZONTAL_SLIDER));
          }
          else{
            SetCursor(get_art_container().Get(Cursor::RESIZE_WE));
          }
        }
      });

    events::on_paint(this, [this](){
      PrepareBitmap();
      wxPaintDC paintDC(this);
      paintDC.DrawBitmap(m_bitmap, 0, 0);
    });

    events::on_resize(this, [this](){Refresh();});
    events::no_op_erase_background(this);
  }

  ~DualSliderImpl(){
    delete m_background;
  }

  Interval GetSelectedInterval() const override{
    return m_range.Constrain(make_interval(floored(m_v1), floored(m_v2)));
  }

  void MouseDown(const IntPoint& pos, bool special){
    if (m_mouse.HasCapture()){
      return;
    }
    int x = pos.x;
    m_special = special;
    int handle = WhichHandle(x);
    if (handle == 2){
      m_anchor = x;
      m_anchorV1 = m_v1;
      m_anchorV2 = m_v2;
    }
    m_which = handle;

    if (m_which != 2){
      UpdateFromPos(m_range.Constrain(pos_to_value(x, GetSize().
            GetWidth(), m_range)), m_which);
    }
    else {
      UpdateFromAnchor(x);
    }
    m_mouse.Capture();
    Refresh();
    SendSliderChangeEvent();
  }

  void PrepareBitmap(){
    IntSize sz(to_faint(GetSize()));
    Bitmap bmp(sz);
    m_background->Draw(bmp, sz, SliderDir::HORIZONTAL);

    // Fixme: Why +1?
    auto selection = make_interval(value_to_pos(m_v1, sz.w, m_range) + 1,
      value_to_pos(m_v2, sz.w, m_range) + 1);

    auto highlightColor = Color(100, 100, 255, 100);
    auto edgeSettings = solid_1px(color_white);

    const auto ySpan = std::make_pair(1, sz.h - 2);

    if (selection.Delta() != 0){
      // Fixme: Blended rectangle instead of Bitmap?
      Bitmap marked(IntSize(selection.Delta(), ySpan.second), highlightColor);
      blend(offsat(marked, {selection.Lower(),ySpan.first}), onto(bmp));

      draw_vline(bmp, selection.Lower(), ySpan, edgeSettings);
      draw_vline(bmp, selection.Upper(), ySpan, edgeSettings);
      draw_vline(bmp, selection.Mid(), ySpan, with_long_dash(edgeSettings));
    }
    m_bitmap = to_wx_bmp(bmp);
  }

  void SendSliderChangeEvent(){
    DualSliderEvent newEvent(GetSelectedInterval(), m_special);
    newEvent.SetEventObject(this);
    ProcessEvent(newEvent);
  }

  void SetSelectedInterval(const Interval& interval) override{
    m_v1 = interval.Lower();
    m_v2 = interval.Upper();
    Refresh();
  }

  void SetBackground(const SliderBackground& bg) override{
    delete m_background;
    m_background = bg.Clone();
    Refresh();
  }

  void UpdateFromAnchor(int x){
    const int dx = m_anchor - x;
    const coord dv = pos_to_value(dx, GetSize().GetWidth(), m_range);
    m_v1 = m_anchorV1 - dv;
    m_v2 = m_anchorV2 - dv;
    double minValue = std::min(m_v1, m_v2);
    double maxValue = std::max(m_v1, m_v2);
    if (minValue < m_range.Lower()){
      m_v1 += m_range.Lower() - minValue;
      m_v2 += m_range.Lower() - minValue;
    }
    if (maxValue > m_range.Upper()){
      m_v1 -= maxValue - m_range.Upper();
      m_v2 -= maxValue - m_range.Upper();
    }
  }

  void UpdateFromPos(double value, int which){
    if (which == 0){
      m_v1 = value;
    }
    else if (which == 1){
      m_v2 = value;
    }
  }

  int WhichHandle(int x){
    IntSize sz(to_faint(GetSize()));
    const int x0 = value_to_pos(m_v1, sz.w, m_range);
    const int x1 = value_to_pos(m_v2, sz.w, m_range);
    int dx0 = std::abs(x0 - x);
    int dx1 = std::abs(x1 - x);
    int xMid = x0 + (x1 - x0) / 2;
    int dxA = std::abs(xMid - x);

    if (dx0 <= dx1 && (dx0 <= dxA || dxA > 5)){
      return 0;
    }
    else if (dx1 < dx0 && (dx1 < dxA || dxA > 5)){
      return 1;
    }
    else {
      return 2;
    }
  }

  int m_anchor; // For moving the entire area
  double m_anchorV1;
  double m_anchorV2;
  SliderBackground* m_background;
  wxBitmap m_bitmap;
  MouseCapture m_mouse;
  ClosedIntRange m_range;
  double m_v1;
  double m_v2;
  int m_which;
  bool m_special = false;
};

DualSliderEvent::DualSliderEvent(const Interval& interval, bool special)
  : wxCommandEvent(FAINT_DUAL_SLIDER_CHANGE, -1),
    m_interval(interval),
    m_special(special)
{}

wxEvent* DualSliderEvent::Clone() const{
  return new DualSliderEvent(*this);
}

Interval DualSliderEvent::GetSelectedInterval() const{
  return m_interval;
}

bool DualSliderEvent::Special() const{
  return m_special;
}

DualSlider* DualSlider::Create(wxWindow* parent,
  const ClosedIntRange& range,
  const Interval& startInterval,
  const SliderBackground& bg,
  const IntSize& initialSize)
{
  auto* s = new DualSliderImpl(parent, range, startInterval, bg);
  s->SetInitialSize(to_wx(initialSize));
  return s;
}

} // namespace

namespace faint{ namespace events{

void on_slider_change(DualSlider* s, const interval_func& f){
  bind_fwd(s, EVT_FAINT_DUAL_SLIDER_CHANGE,
    [f](const DualSliderEvent& e){
      f(e.GetSelectedInterval(), e.Special());
    });
}

void on_dual_slider_change(window_t w, const interval_func& f){
  bind_fwd(w.w, EVT_FAINT_DUAL_SLIDER_CHANGE,
    [f](const DualSliderEvent& e){
      f(e.GetSelectedInterval(), e.Special());
    });
}

}}
