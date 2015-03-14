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
#include "bitmap/bitmap-templates.hh"
#include "commands/function-cmd.hh"
#include "gui/layout.hh"
#include "gui/dual-slider.hh"
#include "gui/slider-histogram-background.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/gui-util.hh"
#include "util-wx/key-codes.hh"
#include "util-wx/layout-wx.hh"
#include "util/command-util.hh"
#include "util/optional.hh"

namespace faint{

class ColorBalanceDialog : public wxDialog {
public:
  ColorBalanceDialog(wxWindow& parent, DialogFeedback& feedback)
    : wxDialog(&parent, wxID_ANY, "Color Balance",
      wxDefaultPosition, wxDefaultSize,
      wxDEFAULT_DIALOG_STYLE | wxWANTS_CHARS | wxRESIZE_BORDER),
      m_bitmap(feedback.GetBitmap()),
      m_enablePreview(nullptr),
      m_feedback(feedback),
      m_redSlider(nullptr),
      m_greenSlider(nullptr),
      m_blueSlider(nullptr)
  {
    // Create the member-controls in intended tab-order (placement follows)
    m_enablePreview = create_checkbox(this, "&Preview", true,
      [&](){
        if (PreviewEnabled()){
          UpdatePreview();
        }
        else {
          ResetPreview();
        }
      });

    auto create_color_slider = [&](const ColRGB& bg,
      const std::vector<int>& histogram)
    {
      return create_dual_slider(this,
        fractional_bounded_interval<color_range_t>(0.2, 0.8),
        SliderHistogramBackground(histogram, bg),
        ui::horizontal_slider_size);
    };

    m_redSlider = create_color_slider(ui::nice_red,red_histogram(m_bitmap));
    m_greenSlider = create_color_slider(ui::nice_green, green_histogram(m_bitmap));
    m_blueSlider = create_color_slider(ui::nice_blue, blue_histogram(m_bitmap));

    // Outer-most sizer
    using namespace layout;
    SetSizerAndFit(create_column({
      create_row({raw(m_enablePreview)}),
      grow(create_row({grow(m_redSlider)})),
      grow(create_row({grow(m_greenSlider)})),
      grow(create_row({grow(m_blueSlider)})),
      center(create_row_no_pad({
         make_default(this, create_ok_button(this)),
         create_cancel_button(this)}))
    }));

    center_over_parent(this);

    set_accelerators(this, {
      {key::F5, [=](){UpdatePreview();}},
      {key::P, [=](){toggle(m_enablePreview);}
      }});

    m_redSlider->SetFocus();
    UpdatePreview();

    events::on_dual_slider_change(this,
      [&](const Interval& interval, bool special){
        if (special){
          m_redSlider->SetSelectedInterval(interval);
          m_greenSlider->SetSelectedInterval(interval);
          m_blueSlider->SetSelectedInterval(interval);
        }
        if (PreviewEnabled()){
          UpdatePreview();
        }
      });

  }

  BitmapCommand* GetCommand(){
    color_range_t red = GetRange(m_redSlider);
    color_range_t green = GetRange(m_greenSlider);
    color_range_t blue = GetRange(m_blueSlider);
    return get_function_command("Color balance",
      [=](Bitmap& bmp){color_balance(bmp, red, green, blue);});
  }

private:
  color_range_t GetRange(DualSlider* slider){
    return color_range_t(slider->GetSelectedInterval());
  }

  bool PreviewEnabled() const{
    return get(m_enablePreview);
  }

  void ResetPreview(){
    m_feedback.SetBitmap(m_bitmap);
  }

  void UpdatePreview(){
    color_range_t red = GetRange(m_redSlider);
    color_range_t green = GetRange(m_greenSlider);
    color_range_t blue = GetRange(m_blueSlider);
    m_feedback.SetBitmap(onto_new(color_balance, m_bitmap, red, green, blue));
  }

  Bitmap m_bitmap;
  wxCheckBox* m_enablePreview;
  DialogFeedback& m_feedback;
  DualSlider* m_redSlider;
  DualSlider* m_greenSlider;
  DualSlider* m_blueSlider;
};

Optional<BitmapCommand*> show_color_balance_dialog(wxWindow& parent,
  DialogFeedback& feedback)
{
  ColorBalanceDialog dlg(parent, feedback);
  if (dlg.ShowModal() == wxID_OK){
    return option(dlg.GetCommand());
  }
  return no_option();
}

} // namespace
