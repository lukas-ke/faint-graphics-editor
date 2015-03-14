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
#include "wx/dialog.h"
#include "bitmap/bitmap.hh"
#include "bitmap/bitmap-templates.hh"
#include "bitmap/filter.hh"
#include "gui/accelerator-entry.hh"
#include "gui/layout.hh"
#include "gui/slider.hh"
#include "gui/command-dialog.hh"
#include "util-wx/fwd-bind.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/key-codes.hh"
#include "util-wx/layout-wx.hh"
#include "util/command-util.hh"
#include "util/optional.hh"

namespace faint{

static bool enable_preview_default(const Bitmap& bmp){
  return (bmp.m_w * bmp.m_h) < (1024 * 1024);
}

class BrightnessContrastDialog : public wxDialog {
public:
  BrightnessContrastDialog(wxWindow& parent, DialogFeedback& feedback)
    : wxDialog(&parent,
      wxID_ANY,
      "Brightness and Contrast",
      wxDefaultPosition,
      wxDefaultSize,
      wxDEFAULT_DIALOG_STYLE | wxWANTS_CHARS | wxRESIZE_BORDER),
      m_bitmap(feedback.GetBitmap()),
      m_feedback(feedback)
  {
    using namespace layout;

    // Create the member-controls in intended tab-order (placement follows)
    m_enablePreview = create_checkbox(this, "&Preview",
      enable_preview_default(m_bitmap),
      [&](){
        if (!PreviewEnabled()){
          ResetPreview();
        }
        else {
          UpdatePreview();
        }
      });

    auto lblBrightness = label(this, "&Brightness");
    m_brightnessSlider = create_slider(this,
      BoundedInt::Mid(min_t(0), max_t(100)),
      SliderDir::HORIZONTAL,
      BorderedSliderMarker(),
      SliderMidPointBackground(),
      ui::horizontal_slider_size);

    auto lblContrast = label(this, "&Contrast");
    m_contrastSlider = create_slider(this,
      BoundedInt::Mid(min_t(0), max_t(100)),
      SliderDir::HORIZONTAL,
      BorderedSliderMarker(),
      SliderMidPointBackground(),
      ui::horizontal_slider_size);

    make_uniformly_sized({lblContrast, lblBrightness});

    SetSizerAndFit(create_column({
      create_row({raw(m_enablePreview)}),
      grow(create_row({
        {raw(lblBrightness), 0, wxALIGN_CENTER_VERTICAL},
        {m_brightnessSlider, 1, wxEXPAND}})),
      grow(create_row({
        {raw(lblContrast), 0, wxALIGN_CENTER_VERTICAL},
        {m_contrastSlider, 1, wxEXPAND}})),
      center(create_row_no_pad({
        make_default(this, create_ok_button(this)),
          create_cancel_button(this)}))
    }));

    center_over_parent(this);

    set_accelerators(this, {
      {key::F5, [=](){UpdatePreview();}},
      {key::P, [=](){
         toggle(m_enablePreview);
        }
      }});

    m_brightnessSlider->SetFocus();

    events::on_slider_change(this,
      [&](int){
        if (PreviewEnabled()){
          UpdatePreview();
        }
      });
  }

  BitmapCommand* GetCommand(){
    return get_brightness_and_contrast_command(GetValues());
  }

  bool ValuesModified() const{
    return m_contrastSlider->GetValue() != 50 ||
      m_brightnessSlider->GetValue() != 50;
  }

private:
  brightness_contrast_t GetValues() const{
    return brightness_contrast_t(m_brightnessSlider->GetValue() / 50.0 - 1.0,
      m_contrastSlider->GetValue() / 50.0);
  }

  bool PreviewEnabled() const{
    return get(m_enablePreview);
  }

  void ResetPreview(){
    m_feedback.SetBitmap(m_bitmap);
  }

  void UpdatePreview(){
    m_feedback.SetBitmap(brightness_and_contrast(m_bitmap, GetValues()));
  }

  Bitmap m_bitmap;
  Slider* m_brightnessSlider = nullptr;
  Slider* m_contrastSlider = nullptr;
  wxCheckBox* m_enablePreview = nullptr;
  DialogFeedback& m_feedback;
};

Optional<BitmapCommand*> show_brightness_contrast_dialog(wxWindow& parent,
  DialogFeedback& feedback)
{
  BrightnessContrastDialog dlg(parent, feedback);
  if (dlg.ShowModal() == wxID_OK && dlg.ValuesModified()){
    return option(dlg.GetCommand());
  }
  return no_option();
}

} // namespace
