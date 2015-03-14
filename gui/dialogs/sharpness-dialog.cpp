// -*- coding: us-ascii-unix -*-
// Copyright 2013 Lukas Kemmer
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
#include "wx/sizer.h"
#include "bitmap/bitmap-templates.hh"
#include "bitmap/filter.hh"
#include "bitmap/gaussian-blur.hh"
#include "commands/function-cmd.hh"
#include "gui/slider.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/gui-util.hh"
#include "util-wx/key-codes.hh"
#include "util-wx/layout-wx.hh"
#include "util/command-util.hh"
#include "util/optional.hh"

namespace faint{

static bool enable_preview_default(const Bitmap& bmp){
  return (bmp.m_w * bmp.m_h) < (1024 * 1024);
}

class SharpnessDialog : public wxDialog {
public:
  SharpnessDialog(wxWindow& parent, DialogFeedback& feedback)
    : wxDialog(&parent, wxID_ANY, "Sharpness", wxDefaultPosition, wxDefaultSize,
        wxDEFAULT_DIALOG_STYLE | wxWANTS_CHARS | wxRESIZE_BORDER),
      m_bitmap(feedback.GetBitmap()),
      m_enablePreview(nullptr),
      m_feedback(feedback),
      m_sharpnessSlider(nullptr)
  {
    // Create the member-controls in intended tab-order (placement follows)
    m_enablePreview = create_checkbox(this, "&Preview",
      enable_preview_default(m_bitmap),
      [&](){
        if (PreviewEnabled()){
          UpdatePreview();
        }
        else{
          ResetPreview();
        }
      });

    m_sharpnessSlider = create_slider(this,
      BoundedInt::Mid(min_t(-100), max_t(100)),
      SliderDir::HORIZONTAL,
      BorderedSliderMarker(),
      SliderMidPointBackground(),
      IntSize(200, 20));

    using namespace layout;
    set_sizer(this, create_column({
      create_row({raw(m_enablePreview)}),
      grow(create_row({grow(m_sharpnessSlider)})),
      center(create_row_no_pad({
        make_default(this, create_ok_button(this)),
        create_cancel_button(this)}))
    }));

    center_over_parent(this);

    set_accelerators(this, {
      {key::F5, [=](){UpdatePreview();}},
      {key::P, [=](){
         toggle(m_enablePreview);
        }}});

    events::on_slider_change(this,
      [this](int){
        if (PreviewEnabled()){
          UpdatePreview();
        }});
  }

  BitmapCommand* GetCommand(){
    coord sharpness = GetSharpness();
    if (sharpness < 0){
      return get_function_command("Blur",
        [=](Bitmap& bmp){bmp = gaussian_blur_fast(bmp, -sharpness);});
    }
    else{
      return get_function_command("Sharpen",
        [=](Bitmap& bmp){bmp = unsharp_mask_fast(bmp, sharpness);});
    }
  }

  bool ValidSharpness(){
    return m_sharpnessSlider->GetValue() != 0;
  }

private:
  coord GetSharpness() const{
    return m_sharpnessSlider->GetValue() / 10.0;
  }

  bool PreviewEnabled() const{
    return get(m_enablePreview);
  }

  void ResetPreview(){
    m_feedback.SetBitmap(m_bitmap);
  }

  void UpdatePreview(){
    if (ValidSharpness()){
      double sharpness = GetSharpness();
      if (sharpness < 0){
        m_feedback.SetBitmap(gaussian_blur_fast(m_bitmap, -sharpness));
      }
      else {
        m_feedback.SetBitmap(unsharp_mask_fast(m_bitmap, sharpness));
      }
    }
    else{
      m_feedback.SetBitmap(m_bitmap);
    }
  }

  Bitmap m_bitmap;
  wxCheckBox* m_enablePreview;
  DialogFeedback& m_feedback;
  Slider* m_sharpnessSlider;
};

Optional<BitmapCommand*> show_sharpness_dialog(wxWindow& parent,
  DialogFeedback& feedback)
{
  SharpnessDialog dlg(parent, feedback);
  if (dlg.ShowModal() == wxID_OK && dlg.ValidSharpness()){
    return option(dlg.GetCommand());
  }
  return no_option();
}

} // namespace
