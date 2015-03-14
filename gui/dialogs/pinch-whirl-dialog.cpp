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

#include <vector>
#include "wx/dialog.h"
#include "wx/sizer.h"
#include "bitmap/bitmap-templates.hh"
#include "gui/layout.hh"
#include "gui/slider.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/gui-util.hh"
#include "util-wx/key-codes.hh"
#include "util-wx/layout-wx.hh"
#include "util/command-util.hh"
#include "util/optional.hh"

namespace faint{

class PinchWhirlDialog : public wxDialog {
public:
  PinchWhirlDialog(wxWindow& parent, DialogFeedback& feedback)
    : wxDialog(&parent, wxID_ANY, "Pinch and Whirl",
        wxDefaultPosition,
        wxDefaultSize,
        wxDEFAULT_DIALOG_STYLE | wxWANTS_CHARS | wxRESIZE_BORDER),
      m_bitmap(feedback.GetBitmap()),
      m_pinchSlider(nullptr),
      m_whirlSlider(nullptr),
      m_enablePreviewCheck(nullptr),
      m_feedback(feedback)
  {
    using namespace layout;

    // Create member controls in intended tab-order (placement follows)
    m_enablePreviewCheck = create_checkbox(this, "&Preview", true,
      [&](){
        if (PreviewEnabled()){
          UpdatePreview();
        }
        else{
          ResetPreview();
        }
      });

    auto lblPinch = label(this, "Pinch");
    m_pinchSlider = create_slider(this,
      BoundedInt::Mid(min_t(-100), max_t(100)),
      SliderDir::HORIZONTAL,
      BorderedSliderMarker(),
      SliderMidPointBackground(),
      ui::horizontal_slider_size);

    auto lblWhirl = label(this, "Whirl");
    m_whirlSlider = create_slider(this,
      BoundedInt::Mid(min_t(-360), max_t(360)),
      SliderDir::HORIZONTAL,
      BorderedSliderMarker(),
      SliderMidPointBackground(),
      ui::horizontal_slider_size);

    make_uniformly_sized({lblPinch, lblWhirl});

    events::on_slider_change(this,
      [&](int){
        if (PreviewEnabled()){
          UpdatePreview();
        }});

    set_sizer(this, create_column({
      create_row({raw(m_enablePreviewCheck)}),
      grow(create_row({
       {raw(lblPinch), 0, wxALIGN_CENTER_VERTICAL},
        grow(m_pinchSlider)})),
      grow(create_row({
        {raw(lblWhirl), 0, wxALIGN_CENTER_VERTICAL},
         grow(m_whirlSlider)})),
      center(create_row_no_pad({
        make_default(this, create_ok_button(this)),
          create_cancel_button(this)}))
    }));

    center_over_parent(this);

    set_accelerators(this, {
      {key::F5, [=](){UpdatePreview();}},
      {key::P, [=](){toggle(m_enablePreviewCheck);}}
    });
  }

  BitmapCommand* GetCommand(){
    return get_pinch_whirl_command(GetPinchValue(), GetWhirlValue());
  }

  bool ValuesModified() const{
    return m_pinchSlider->GetValue() != 0 || m_whirlSlider->GetValue() != 0;
  }

private:
  coord GetPinchValue() const{
    return m_pinchSlider->GetValue() / 50.0;
  }

  Angle GetWhirlValue() const{
    return Angle::Deg(m_whirlSlider->GetValue());
  }

  bool PreviewEnabled() const{
    return get(m_enablePreviewCheck);
  }

  void ResetPreview(){
    m_feedback.SetBitmap(m_bitmap);
  }

  void UpdatePreview(){
    m_feedback.SetBitmap(onto_new(filter_pinch_whirl, m_bitmap,
      GetPinchValue(), GetWhirlValue()));
  }

  Bitmap m_bitmap;
  Slider* m_pinchSlider;
  Slider* m_whirlSlider;
  wxCheckBox* m_enablePreviewCheck;
  DialogFeedback& m_feedback;
};

Optional<BitmapCommand*> show_pinch_whirl_dialog(wxWindow& parent,
  DialogFeedback& feedback)
{
  PinchWhirlDialog dlg(parent, feedback);
  if (dlg.ShowModal() == wxID_OK && dlg.ValuesModified()){
    return option(dlg.GetCommand());
  }
  return no_option();
}

} // namespace
