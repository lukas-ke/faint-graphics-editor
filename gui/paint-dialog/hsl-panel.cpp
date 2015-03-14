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
#include "wx/dcclient.h" // wxPaintDC
#include "wx/textctrl.h"
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/draw.hh"
#include "bitmap/pattern.hh"
#include "geo/int-range.hh"
#include "geo/int-rect.hh"
#include "gui/layout.hh"
#include "gui/mouse-capture.hh"
#include "gui/paint-dialog/hsl-panel.hh"
#include "gui/slider.hh"
#include "gui/slider-alpha-background.hh"
#include "gui/static-bitmap.hh"
#include "text/formatting.hh"
#include "util-wx/bind-event.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/fwd-bind.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/gui-util.hh"
#include "util-wx/placement.hh"
#include "util-wx/make-event.hh"
#include "util/color-bitmap-util.hh"

namespace faint{

MAKE_FAINT_COMMAND_EVENT(PICKED_HUE_SAT);

class LightnessBackground : public SliderBackground{
public:
  explicit LightnessBackground(const HS& hueSat)
    : m_hueSat(hueSat)
  {}

  void Draw(Bitmap& bmp, const IntSize& size, SliderDir) override{
    if (!bitmap_ok(m_bitmap) || m_bitmap.GetSize() != size){
      InitializeBitmap(size);
    }
    blit(at_top_left(m_bitmap), onto(bmp));
  }
  SliderBackground* Clone() const override{
    return new LightnessBackground(*this);
  }
private:
  void InitializeBitmap(const IntSize& size){
    m_bitmap = lightness_gradient_bitmap(m_hueSat, size);
  }
  Bitmap m_bitmap;
  HS m_hueSat;
};

class HueSatPicker : public wxPanel {
public:
  HueSatPicker(wxWindow* parent) :
    wxPanel(parent, wxID_ANY),
    m_hslSize(241,241),
    m_hueSat(0.0,0.0),
    m_mouse(this)
  {
    SetBackgroundStyle(wxBG_STYLE_PAINT); // Prevent flicker on full refresh
    SetInitialSize(m_hslSize);
    InitializeBitmap();

    events::on_paint(this, [this](){
      wxPaintDC dc(this);
      dc.DrawBitmap(m_bmp, 0, 0);

      // Draw the marker
      dc.SetPen(wxPen(wxColour(0,0,0), 1, wxPENSTYLE_SOLID));
      int x = floored((m_hueSat.h / 360.0) * 240.0);
      int y = floored(240 - m_hueSat.s * 240);
      dc.DrawLine(x, y - 10, x, y + 10);
      dc.DrawLine(x - 10, y, x + 10, y);
    });

    events::on_mouse_left_down(this,
      [this](const IntPoint& pos){
        SetFromPos(pos);
        m_mouse.Capture();
      });

    events::on_mouse_left_up(this, releaser(m_mouse));

    events::on_mouse_motion(this,
      [this](const IntPoint& pos){
        if (m_mouse.HasCapture()){
          SetFromPos(pos);
        }
      });
  }

  bool AcceptsFocus() const override{
    return false;
  }

  bool AcceptsFocusFromKeyboard() const override{
    return false;
  }

  HS GetValue() const{
    return m_hueSat;
  }

  void Set(const HS& hueSat){
    m_hueSat = hueSat;
    Refresh();
  }

private:
  void InitializeBitmap(){
    m_bmp = to_wx_bmp(hue_saturation_color_map(to_faint(m_hslSize)));
  }

  void SetFromPos(const IntPoint& pos){
    int viewHue = std::max(0, std::min(pos.x, 240));
    m_hueSat.h = std::min((viewHue / 240.0) * 360.0, 359.0); // Fixme
    m_hueSat.s = 1.0 - std::max(0, std::min(pos.y, 240)) / 240.0;
    Refresh();
    wxCommandEvent newEvent(EVT_PICKED_HUE_SAT);
    newEvent.SetEventObject(this);
    ProcessEvent(newEvent);
  }

  wxBitmap m_bmp;
  wxSize m_hslSize;
  HS m_hueSat;
  MouseCapture m_mouse;
};

class PaintPanel_HSL::PaintPanel_HSL_Impl : public wxPanel{
public:
  PaintPanel_HSL_Impl(wxWindow* parent)
    : wxPanel(parent, wxID_ANY)
  {
    m_hueSatPicker = new HueSatPicker(this);
    set_pos(m_hueSatPicker, IntPoint::Both(panel_padding));
    auto lblHue = create_label(this, "&Hue");
    m_hueTxt = BindKillFocus(CreateTextControl({min_t(0),max_t(240)},
      below(m_hueSatPicker)));
    PlaceLabel(lblHue, m_hueTxt, true);

    auto lblSat = create_label(this, "&Sat");
    m_saturationTxt = BindKillFocus(CreateTextControl({min_t(0),max_t(240)},
      below(m_hueTxt)));
    PlaceLabel(lblSat, m_saturationTxt, false);

    m_lightnessSlider = create_slider(this,
      BoundedInt::Mid(min_t(0), max_t(240)),
      SliderDir::VERTICAL,
      BorderedSliderMarker(),
      LightnessBackground(m_hueSatPicker->GetValue()),
      IntSize(20, 240));
    set_pos(m_lightnessSlider, to_the_right_of(m_hueSatPicker));

    m_alphaSlider = create_slider(this,
      BoundedInt::Mid(min_t(0), max_t(255)),
      SliderDir::VERTICAL,
      BorderedSliderMarker(),
      AlphaBackground(ColRGB(128,128,128)),
      IntSize(20,255));
    set_pos(m_alphaSlider, to_the_right_of(m_lightnessSlider));

    auto lblLightness = create_label(this, "&Lightness");
    m_lightnessTxt = BindKillFocus(CreateTextControl({min_t(0),max_t(240)}));
    auto lightnessTxtPos = get_pos(m_lightnessSlider);
    lightnessTxtPos.x += get_width(m_lightnessSlider) - get_width(m_lightnessTxt);
    lightnessTxtPos.y = get_pos(m_hueTxt).y;
    set_pos(m_lightnessTxt, lightnessTxtPos);
    PlaceLabel(lblLightness, m_lightnessTxt);

    m_colorBitmap = new StaticBitmap(this, Bitmap({120,120}, color_black));
    set_pos(m_colorBitmap, to_the_right_of(m_alphaSlider));

    // RGBA text-boxes
    auto lblRed = create_label(this, "&R");
    m_redTxt = BindKillFocus(CreateTextControl({min_t(0), max_t(255)}));
    auto redPos = get_pos(m_colorBitmap);
    redPos.x = redPos.x + get_width(m_colorBitmap) - get_width(m_redTxt);
    redPos.y += get_height(m_colorBitmap) + 5;
    set_pos(m_redTxt, redPos);
    PlaceLabel(lblRed, m_redTxt);

    auto lblGreen = create_label(this, "&G");
    IntPoint greenPos(redPos.x, redPos.y + get_height(m_redTxt) + 5);
    m_greenTxt = BindKillFocus(CreateTextControl({min_t(0), max_t(255)},
        greenPos));
    PlaceLabel(lblGreen, m_greenTxt);

    auto lblBlue = create_label(this, "&B");
    IntPoint bluePos(greenPos.x, greenPos.y + get_height(m_greenTxt) + 5);
    m_blueTxt = BindKillFocus(CreateTextControl({min_t(0), max_t(255)}, bluePos));
    PlaceLabel(lblBlue, m_blueTxt);

    auto lblAlpha = create_label(this, "&A");
    IntPoint alphaPos(bluePos.x, bluePos.y + get_height(m_blueTxt) + 5);
    m_alphaTxt = BindKillFocus(CreateTextControl({min_t(0), max_t(255)},
      alphaPos));
    PlaceLabel(lblAlpha, m_alphaTxt);

    SetInitialSize(wxSize(right_side(m_colorBitmap) + panel_padding,
      bottom(m_saturationTxt) + panel_padding));
    SetColor(Color(0,0,128,255));

    UpdateRGBA();
    UpdateHSL();
    UpdateColorBitmap();

    events::on_slider_change(m_lightnessSlider,
      [this](int lightness){
        HSL hsl(m_hueSatPicker->GetValue(), lightness / 240.0);
        m_alphaSlider->SetBackground(AlphaBackground(to_rgb(hsl)));
        UpdateColorBitmap();
        set_number_text(m_lightnessTxt, lightness, Signal::NO);
        UpdateRGBA();
      });

    events::on_slider_change(m_alphaSlider,
      [this](int alpha){
        UpdateColorBitmap();
        set_number_text(m_alphaTxt, alpha, Signal::NO);
      });

    bind(this, EVT_PICKED_HUE_SAT,
      [this](){
        HS hueSat(m_hueSatPicker->GetValue());
        m_lightnessSlider->SetBackground(LightnessBackground(hueSat));
        ColRGB rgb(to_rgb(HSL(hueSat, m_lightnessSlider->GetValue() / 240.0))); // Fixme: Nasty conversion
        m_alphaSlider->SetBackground(AlphaBackground(rgb));
        UpdateColorBitmap();
        int hue = static_cast<int>((hueSat.h / 360) * 240); // Fixme
        set_number_text(m_hueTxt, hue, Signal::NO);

        int saturation = static_cast<int>(hueSat.s * 40);
        set_number_text(m_saturationTxt, saturation, Signal::NO);
        UpdateRGBA();
      });

    // Fixme: Bind each text-control separately instead
    bind_fwd(this, wxEVT_COMMAND_TEXT_UPDATED,
      [this](wxCommandEvent& evt){
        auto* ctrl = dynamic_cast<wxTextCtrl*>(evt.GetEventObject());
        IntRange range(m_ranges[ctrl]);

        int value = range.Constrain(parse_int_value(ctrl,0));

        if (ctrl == m_hueTxt){
          HS hs = m_hueSatPicker->GetValue();
          hs.h = std::min((value / 240.0) * 360.0, 359.0); // Fixme
          m_hueSatPicker->Set(hs);
          m_lightnessSlider->SetBackground(LightnessBackground(hs));
          UpdateColorBitmap();
        }
        else if (ctrl == m_saturationTxt){
          HS hs = m_hueSatPicker->GetValue();
          hs.s = value / 240.0;
          m_hueSatPicker->Set(hs);
          m_lightnessSlider->SetBackground(LightnessBackground(hs));
          UpdateColorBitmap();
        }
        else if (ctrl == m_lightnessTxt){
          m_lightnessSlider->SetValue(value);
          m_alphaSlider->SetBackground(AlphaBackground(strip_alpha(GetColor())));
          UpdateColorBitmap();
        }
        else if (ctrl == m_alphaTxt){
          m_alphaSlider->SetValue(value);
          UpdateColorBitmap();
        }
        else if (ctrl == m_redTxt || ctrl == m_greenTxt || ctrl == m_blueTxt){
          ColRGB rgb(rgb_from_ints(parse_int_value(m_redTxt, 0),
              parse_int_value(m_greenTxt, 0),
              parse_int_value(m_blueTxt, 0)));
          HSL hsl(to_hsl(rgb));
          m_hueSatPicker->Set(hsl.GetHS());
          m_lightnessSlider->SetBackground(LightnessBackground(hsl.GetHS()));
          m_lightnessSlider->SetValue(floored(hsl.l * 240.0)); // Fixme: Conversion
          m_alphaSlider->SetBackground(AlphaBackground(rgb));
          UpdateColorBitmap();
          UpdateHSL();
        }
      });
  }

  Color GetColor() const{
    Color c(to_rgba(HSL(m_hueSatPicker->GetValue(),
      m_lightnessSlider->GetValue() / 240.0),
      m_alphaSlider->GetValue()));
    return c;
  }

  void SetColor(const Color& color){
    HSL hsl(to_hsl(strip_alpha(color)));
    m_hueSatPicker->Set(hsl.GetHS());
    m_lightnessSlider->SetBackground(LightnessBackground(hsl.GetHS()));
    m_lightnessSlider->SetValue(floored(hsl.l * 240.0)); // Fixme: Nasty conversion
    m_alphaSlider->SetBackground(AlphaBackground(strip_alpha(color)));
    m_alphaSlider->SetValue(color.a);
    UpdateColorBitmap();
    UpdateRGBA();
    UpdateHSL();
  }
private:
  wxTextCtrl* BindKillFocus(wxTextCtrl* textCtrl){
    events::on_kill_focus(textCtrl,
      [this, textCtrl](){
        // Set the value to 0 if invalid
        wxString text = textCtrl->GetValue();
        if (text.empty() || !text.IsNumber() || text.Contains("-")){
          set_number_text(textCtrl, 0, Signal::YES);
          return;
        }

        int value = parse_int_value(textCtrl, 0);
        IntRange& range(m_ranges[textCtrl]);
        if (!range.Has(value)){
          set_number_text(textCtrl, range.Constrain(value), Signal::YES);
        }
      });
    return textCtrl;
  }

  wxTextCtrl* CreateTextControl(const IntRange& range, const IntPoint& pos){
    wxTextCtrl* t = create_text_control(this, pos, Width(50));
    t->SetMaxLength(3);
    m_ranges[t] = range;
    return t;
  }

  wxTextCtrl* CreateTextControl(const IntRange& range){
    return CreateTextControl(range, to_faint(wxDefaultPosition));
  }

  void UpdateRGBA(){
    Color c(GetColor());
    m_redTxt->ChangeValue(to_wx(str_int(c.r)));
    m_greenTxt->ChangeValue(to_wx(str_int(c.g)));
    m_blueTxt->ChangeValue(to_wx(str_int(c.b)));
    m_alphaTxt->ChangeValue(to_wx(str_int(c.a)));
  }

  void UpdateHSL(){
    Color c(GetColor());
    HSL hsl(to_hsl(strip_alpha(c)));
    m_hueTxt->ChangeValue(to_wx(str_int(truncated((hsl.h / 360.0) * 240.0)))); // Fixme
    m_saturationTxt->ChangeValue(to_wx(str_int(truncated(hsl.s * 240.0)))); // Fixme
    m_lightnessTxt->ChangeValue(to_wx(str_int(truncated(hsl.l * 240.0)))); // Fixme
  }

  void UpdateColorBitmap(){
    Bitmap bmp(color_bitmap(GetColor(), to_faint(m_colorBitmap->GetSize())));
    m_colorBitmap->SetBitmap(bmp);
  }

  void PlaceLabel(wxStaticText* label, wxTextCtrl* control, bool shift=false){
    // Place the label to the left of the control.
    // Note: I previously just had an AddLabel-function, but the label
    // would then end up after the (already-created) control in the
    // tab order. Hence, the label must be created before the
    // control. I tried wxWindows::MoveBeforeInTabOrder, but this did
    // not affect traversal with mnemonics, so e.g. Alt+H for the hue
    // label ("&Hue") would select the saturation text field.
    auto ctrlPos(get_pos(control));
    auto ctrlSize(get_size(control));
    auto lblSize(get_size(label));
    if (shift){
      set_pos(control, IntPoint(ctrlPos.x + lblSize.w + 5, ctrlPos.y));

      set_pos(label, IntPoint(ctrlPos.x,
          ctrlPos.y + ctrlSize.h / 2 - lblSize.h / 2));
    }
    else {
      set_pos(label,
        {ctrlPos.x - lblSize.w - 5,
         ctrlPos.y + ctrlSize.h / 2 - lblSize.h / 2});
    }
  }

  Slider* m_alphaSlider;
  wxTextCtrl* m_alphaTxt;
  wxTextCtrl* m_blueTxt;
  StaticBitmap* m_colorBitmap;
  wxTextCtrl* m_greenTxt;
  HueSatPicker* m_hueSatPicker;
  wxTextCtrl* m_hueTxt;
  Slider* m_lightnessSlider;
  wxTextCtrl* m_lightnessTxt;
  std::map<wxTextCtrl*, IntRange> m_ranges;
  wxTextCtrl* m_redTxt;
  wxTextCtrl* m_saturationTxt;
};

PaintPanel_HSL::PaintPanel_HSL(wxWindow* parent){
  m_impl = new PaintPanel_HSL_Impl(parent);
}

PaintPanel_HSL::~PaintPanel_HSL(){
  m_impl = nullptr; // Deletion handled by wxWidgets
}

Color PaintPanel_HSL::GetColor() const{
  return m_impl->GetColor();
}

void PaintPanel_HSL::SetColor(const Color& color){
  m_impl->SetColor(color);
}

wxWindow* PaintPanel_HSL::AsWindow(){
  return m_impl;
}

} // namespace
