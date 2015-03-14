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

#include "bitmap/bitmap-templates.hh" // onto_new
#include "bitmap/filter.hh"
#include "gui/command-window.hh"
#include "gui/dialog-context.hh"
#include "gui/dual-slider.hh"
#include "gui/layout.hh"
#include "gui/slider-histogram-background.hh"
#include "util/command-util.hh" // get_threshold_command
#include "util-wx/fwd-bind.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/gui-util.hh"
#include "util-wx/key-codes.hh"
#include "util-wx/layout-wx.hh"

namespace faint{

SliderHistogramBackground threshold_histogram_background(const Bitmap& bmp){
  return SliderHistogramBackground(threshold_histogram(bmp), ColRGB(0,0,0));
}

class ThresholdDialog : public CommandWindow{
public:
  ThresholdDialog(const Settings& s)
    : m_dialog(null_dialog())
  {
    m_settings.Set(ts_Fg, s.GetDefault(ts_Fg, Paint(color_black)));
    m_settings.Set(ts_Bg, s.GetDefault(ts_Bg, Paint(color_white)));
  }

  void Show(wxWindow& parent, WindowFeedback& feedback) override{
    m_bitmap = feedback.GetBitmap();
    m_feedback = &feedback;

    auto cancel = [&](){Close();};
    auto ok = [&](){Close(true);};

    m_dialog = create_dialog(parent, "Threshold");

    events::on_close_window(m_dialog, cancel);
    m_enablePreview = create_checkbox(m_dialog, "Preview", true,
      [&](){
        if (get(m_enablePreview)){
          UpdatePreview();
        }
        else{
          ResetPreview();
        }
      });

    m_slider = create_dual_slider(raw(m_dialog),
      fractional_bounded_interval<threshold_range_t>(0.2, 0.8),
      threshold_histogram_background(m_bitmap),
      ui::tall_horizontal_slider_size,
      [&](const Interval&, bool){
        if (get(m_enablePreview)){
          UpdatePreview();
        }
      });

    using namespace layout;
    set_sizer(m_dialog.get(), create_column({
      create_row({raw(m_enablePreview)}),
      grow(create_row({{m_slider, 1, wxEXPAND}})),
      center(create_row_no_pad({make_default(m_dialog.get(),
      create_button(m_dialog.get(), "&OK",ok)),
        create_button(m_dialog.get(), "Cancel", cancel)}))}));

    set_accelerators(raw(m_dialog), {
        {key::P, [&](){toggle(m_enablePreview);}},
        {key::esc, cancel},
        {key::enter, ok}});

    center_over_parent(m_dialog);
    show(m_dialog);
    UpdatePreview();
  }

  bool IsShown() const override{
    return m_dialog != nullptr;
  }

  bool MouseMove(const PosInfo&) override{
    return false;
  }

  bool MouseDown(const PosInfo&) override{
    return false;
  }

  void Draw(FaintDC&, Overlays&, const PosInfo&) override{}

  void UpdateSettings(const Settings& s) override{
    bool updated = m_settings.Update(s);
    if (updated && get(m_enablePreview)){
      UpdatePreview();
    }
  }

  const Settings& GetSettings() const override{
    return m_settings;
  };

private:
  void Close(bool ok=false){
    if (m_dialog != nullptr){
      BitmapCommand* cmd = ok ?
        get_threshold_command(GetCurrentRange(),
          m_settings.Get(ts_Fg),
          m_settings.Get(ts_Bg))
        : nullptr;
      m_dialog.reset(nullptr);
      m_feedback->Closed(cmd);
      m_feedback = nullptr;
    }
  }

  void UpdatePreview(){
    m_feedback->SetBitmap(onto_new(threshold, m_bitmap, GetCurrentRange(),
      m_settings.Get(ts_Fg), m_settings.Get(ts_Bg)));
  }

  void Reinitialize(WindowFeedback& feedback) override{
    m_bitmap = feedback.GetBitmap();
    m_slider->SetBackground(threshold_histogram_background(m_bitmap));
    UpdatePreview();
  }

  threshold_range_t GetCurrentRange() const{
    return threshold_range_t(m_slider->GetSelectedInterval());
  }

  void ResetPreview(){
    m_feedback->SetBitmap(m_bitmap);
  }

  wxCheckBox* m_enablePreview = nullptr;
  DualSlider* m_slider = nullptr;
  unique_dialog_ptr m_dialog;
  Settings m_settings;
  Bitmap m_bitmap;
  WindowFeedback* m_feedback = nullptr;
};

void show_threshold_dialog(DialogContext& c, const Settings& s){
  c.Show(std::make_unique<ThresholdDialog>(s));
}

} // namespace
