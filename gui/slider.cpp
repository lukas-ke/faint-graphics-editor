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

#include "wx/dcclient.h"
#include "wx/panel.h"
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/draw.hh"
#include "geo/int-rect.hh"
#include "geo/line.hh"
#include "gui/mouse-capture.hh"
#include "gui/slider.hh"
#include "util-wx/bind-event.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/fwd-bind.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/system-colors.hh"

namespace faint{

class SliderEvent : public wxCommandEvent{
public:
  explicit SliderEvent(int value);
  wxEvent* Clone() const override;
  int GetValue() const;
private:
  int m_value;
};

extern const wxEventTypeTag<SliderEvent> EVT_FAINT_SLIDER_CHANGE;

Slider::Slider(wxWindow* parent)
  : wxPanel(parent, wxID_ANY)
{}

class SliderImpl : public Slider{
public:
  SliderImpl(wxWindow* parent,
    const BoundedInt& values,
    SliderDir dir,
    SliderMarkerPtr marker,
    SliderBackgroundPtr bg,
    const SliderCursors& cursors)
    : Slider(parent),
      m_background(std::move(bg)),
      m_dir(dir),
      m_marker(std::move(marker)),
      m_mouse(this),
      m_range(values.GetRange()),
      m_value(values.GetValue())
  {
    SetInitialSize(wxSize(20,20)); // Minimum size
    cursors.SetHandleCursor(this, m_dir);

    events::on_mouse_left_down(this,
      [this](const IntPoint& mousePos){
        if (m_mouse.HasCapture()){
          return;
        }
        m_mouse.Capture();
        UpdateFromMousePos(mousePos);
      });

    events::on_mouse_left_up(this,
      [this](const IntPoint&){
        if (m_mouse.HasCapture()){
          m_mouse.Release();
          Refresh();
        }
      });

    events::on_mouse_motion(this,
      [this](const IntPoint& mousePos){
        if (m_mouse.HasCapture()){
          UpdateFromMousePos(mousePos);
        }
      });

    events::on_paint(this,
      [this](){
        PrepareBitmap();
        wxPaintDC paintDC(this);
        paintDC.DrawBitmap(m_bitmap, 0, 0);
      });

    events::on_resize(this, [this](){Refresh();});
    events::no_op_erase_background(this);
  }

  int GetValue() const override{
    return static_cast<int>(m_value);
  }

  void SetBackground(SliderBackgroundPtr background) override{
    m_background = std::move(background);
    Refresh();
  }

  void SetValue(int value) override{
    m_value = value;
    Refresh();
  }

private:
  void PrepareBitmap(){
    wxSize sz(GetSize());
    Bitmap bmp(to_faint(sz));
    m_background->Draw(bmp, to_faint(sz), m_dir);

    int length = m_dir == SliderDir::HORIZONTAL ?
      GetSize().x :
      GetSize().y;

    int pos = value_to_pos(m_value, length, m_range);
    m_marker->Draw(bmp, m_dir, to_faint(sz), pos);
    m_bitmap = to_wx_bmp(bmp);
  }

  void UpdateFromMousePos(const IntPoint& mousePos){
    int pos = m_dir == SliderDir::HORIZONTAL ?
      mousePos.x :
      mousePos.y;

    int length = m_dir == SliderDir::HORIZONTAL ?
      GetSize().x :
      GetSize().y;

    m_value = m_range.Constrain(pos_to_value(pos, length, m_range));
    Refresh();
    SliderEvent newEvent(GetValue());
    newEvent.SetEventObject(this);
    ProcessEvent(newEvent);
  }

  std::unique_ptr<SliderBackground> m_background;
  wxBitmap m_bitmap;
  SliderDir m_dir;
  SliderMarkerPtr m_marker;
  MouseCapture m_mouse;
  ClosedIntRange m_range;
  double m_value;
};

const wxEventType FAINT_SLIDER_CHANGE = wxNewEventType();

SliderEvent::SliderEvent(int value)
  : wxCommandEvent(FAINT_SLIDER_CHANGE, -1),
    m_value(value)
{}

wxEvent* SliderEvent::Clone() const{
  return make_wx<SliderEvent>(*this);
}

int SliderEvent::GetValue() const{
  return m_value;
}

const wxEventTypeTag<SliderEvent> EVT_FAINT_SLIDER_CHANGE(FAINT_SLIDER_CHANGE);

Slider* create_slider(wxWindow* parent,
  const BoundedInt& values,
  SliderDir dir,
  SliderMarkerPtr marker,
  SliderBackgroundPtr background,
  const SliderCursors& sliderCursors,
  const IntSize& initialSize,
  const std::function<void()>& onChange)
{
  return bind(create_slider(parent,
      values, dir,
      std::move(marker),
      std::move(background),
      sliderCursors,
      initialSize),
    EVT_FAINT_SLIDER_CHANGE, std::move(onChange));
}

Slider* create_slider(wxWindow* parent,
  const BoundedInt& values,
  SliderDir dir,
  SliderMarkerPtr marker,
  SliderBackgroundPtr bg,
  const SliderCursors& cursors,
  const IntSize& initialSize)
{
  auto s = make_wx<SliderImpl>(parent,
    values,
    dir,
    std::move(marker),
    std::move(bg),
    cursors);
  s->SetInitialSize(to_wx(initialSize));
  return s;
}

Slider* create_slider(wxWindow* parent,
  const BoundedInt& values,
  SliderDir dir,
  SliderBackgroundPtr bg,
  const SliderCursors& cursors,
  const IntSize& initialSize)
{
  auto s = make_wx<SliderImpl>(parent,
    values,
    dir,
    slider_marker_Line(),
    std::move(bg),
    cursors);
  s->SetInitialSize(to_wx(initialSize));
  return s;
}

Slider* create_slider(wxWindow* parent,
  const BoundedInt& values,
  SliderDir dir,
  SliderBackgroundPtr background,
  const SliderCursors& cursors,
  const IntSize& initialSize,
  std::function<void()>&& onChange)
{
  return bind(create_slider(parent,
      values,
      dir,
      std::move(background),
      cursors,
      initialSize),
    EVT_FAINT_SLIDER_CHANGE, std::move(onChange));
}

Slider* create_slider(wxWindow* parent,
  const BoundedInt& values,
  SliderDir dir,
  SliderMarkerPtr marker,
  SliderBackgroundPtr background,
  const SliderCursors& cursors,
  const IntSize& initialSize,
  std::function<void(int)>&& onChange)
{
  auto s = create_slider(parent,
    values,
    dir,
    std::move(marker),
    std::move(background),
    cursors,
    initialSize);

  events::on_slider_change(s, onChange);
  return s;
}

class LineSliderMarker : public SliderMarker{
public:
  void Draw(Bitmap& bmp,
    SliderDir dir,
    const IntSize& size,
    int pos) const override
  {
    auto line = (dir == SliderDir::HORIZONTAL ?
      IntLineSegment({pos, 1}, {pos, size.h - 2}):
      IntLineSegment({0, pos}, {size.w - 2, pos}));
    draw_line(bmp, line, solid_1px(color_white));
  }
};

SliderMarkerPtr slider_marker_Line(){
  return std::make_unique<LineSliderMarker>();
}

class BorderedSliderMarker : public SliderMarker{
public:
  void Draw(Bitmap& bmp,
  SliderDir dir,
  const IntSize& size,
  int pos) const override
  {
    if (dir == SliderDir::HORIZONTAL){
      draw_vline(bmp, pos, {1, size.h - 2},
        solid_1px(color_white));
      draw_rect(bmp, {IntPoint(pos - 1, 1), IntPoint(pos + 1, size.h - 2)},
        solid_1px(color_dark_shadow_3d()));
    }
    else if (dir == SliderDir::VERTICAL){
      draw_hline(bmp, pos, {1, size.w - 2},
        solid_1px(color_white));
      draw_rect(bmp, {IntPoint(1, pos - 1), IntPoint(size.w - 2, pos + 1)},
        solid_1px(color_dark_shadow_3d()));
    }
    else{
      assert(false);
    }
  }
};

SliderMarkerPtr slider_marker_BorderedLine(){
  return std::make_unique<BorderedSliderMarker>();
}

} // namespace

namespace faint::events{

void on_slider_change(Slider* s, const int_func& f){
  bind_fwd(s, EVT_FAINT_SLIDER_CHANGE,
    [f](const SliderEvent& e){
      f(e.GetValue());
    });
}

void on_slider_change(window_t w, const int_func& f){
  bind_fwd(w.w, EVT_FAINT_SLIDER_CHANGE,
    [f](const SliderEvent& e){
      f(e.GetValue());
    });
}

} // namespace
