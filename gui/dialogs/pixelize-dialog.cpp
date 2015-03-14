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

#include "wx/dialog.h"
#include "wx/sizer.h"
#include "bitmap/bitmap-templates.hh"
#include "gui/slider.hh"
#include "util-wx/gui-util.hh"
#include "util-wx/key-codes.hh"
#include "util-wx/layout-wx.hh"
#include "util-wx/fwd-wx.hh"
#include "util/command-util.hh"
#include "util/optional.hh"

namespace faint{

static bool enable_preview_default(const Bitmap& bmp){
  return area(bmp.GetSize()) < area({2000, 2000});
}

class PixelizeDialog : public wxDialog {
public:
  PixelizeDialog(wxWindow& parent, DialogFeedback& feedback)
    : wxDialog(&parent, wxID_ANY, "Pixelize", wxDefaultPosition, wxDefaultSize,
      wxDEFAULT_DIALOG_STYLE | wxWANTS_CHARS | wxRESIZE_BORDER),
      m_bitmap(feedback.GetBitmap()),
      m_pixelSizeSlider(nullptr),
      m_enablePreview(nullptr),
      m_feedback(feedback)
  {
    using namespace layout;

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
    m_pixelSizeSlider = create_slider(this,
      BoundedInt::Mid(min_t(1), max_t(100)),

      SliderDir::HORIZONTAL,
      BorderedSliderMarker(),
      SliderRectangleBackground(),
      IntSize(200, 20));

    set_sizer(this, create_column({
      create_row({raw(m_enablePreview)}),
      grow(create_row({grow(m_pixelSizeSlider)})),
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
      [&](int){
        if (PreviewEnabled()){
          UpdatePreview();
        }
      });

    m_pixelSizeSlider->SetFocus();

    if (PreviewEnabled()){
      UpdatePreview();
    }
  }

  BitmapCommand* GetCommand(){
    return get_pixelize_command(m_pixelSizeSlider->GetValue());
  }

  bool ValuesModified() const{
    return m_pixelSizeSlider->GetValue() != 1;
  }

private:
  bool PreviewEnabled() const{
    return get(m_enablePreview);
  }

  void ResetPreview(){
    m_feedback.SetBitmap(m_bitmap);
  }

  void UpdatePreview(){
    m_feedback.SetBitmap(onto_new(pixelize, m_bitmap,
      m_pixelSizeSlider->GetValue()));
  }

  Bitmap m_bitmap;
  Slider* m_pixelSizeSlider;
  wxCheckBox* m_enablePreview;
  DialogFeedback& m_feedback;
};

Optional<BitmapCommand*> show_pixelize_dialog(wxWindow& parent,
  DialogFeedback& feedback)
{
  PixelizeDialog dlg(parent, feedback);
  if (dlg.ShowModal() == wxID_OK && dlg.ValuesModified()){
    return option(dlg.GetCommand());
  }
  return no_option();
}

} // namespace
