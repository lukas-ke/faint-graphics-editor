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
#include "bitmap/color.hh"
#include "bitmap/mask.hh"
#include "bitmap/paint.hh"
#include "gui/command-window.hh"
#include "gui/dialog-context.hh"
#include "gui/slider.hh"
#include "gui/slider-alpha-background.hh"
#include "gui/ui-constants.hh"
#include "util/accessor.hh"
#include "util-wx/fwd-bind.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/gui-util.hh"
#include "util-wx/key-codes.hh"
#include "util-wx/layout-wx.hh"
#include "util/command-util.hh" // get_set_alpha_command

namespace faint{

static int get_initial_alpha(const Bitmap& bmp){
  // Initialize the alpha from the top left pixel of the target
  // bitmap
  return bitmap_ok(bmp) ? static_cast<int>(get_color_raw(bmp, 0, 0).a) :
    255;
}

static Optional<Color> get_mask_color(const Settings& s,
  const WindowFeedback& feedback)
{
  bool maskedBg = s.Get(ts_BackgroundStyle) == BackgroundStyle::MASKED;
  if (!feedback.FloatingSelection() || !maskedBg){
    return {};
  }

  const auto& bg = s.Get(ts_Bg);
  return bg.IsColor() ?
    option(bg.GetColor()) :
    no_option();
}

static void set_preview(const Bitmap& src,
  uchar alpha,
  WindowFeedback& feedback,
  const Settings& s)
{
  get_mask_color(s, feedback).Visit(
    [&](const Color& c){
      const auto mask = mask_not_color(src, c);
      feedback.SetPreview(onto_new(set_alpha_masked, src, alpha, mask));
    },
    [&](){
      feedback.SetPreview(onto_new(set_alpha, src, alpha));
    });
}

class AlphaDialog : public CommandWindow{
public:
  explicit AlphaDialog(const SliderCursors& sliderCursors)
    : m_dialog(null_dialog()),
      m_sliderCursors(sliderCursors)
  {
    m_settings.Set(ts_AlphaBlending, false);
    m_settings.Set(ts_Bg, Paint(color_white));
    m_settings.Set(ts_BackgroundStyle, BackgroundStyle::MASKED);
  }

  void Show(wxWindow& parent,
    const Settings& settings,
    WindowFeedback& feedback) override
  {
    m_settings.Update(settings);
    auto cancel = [&](){Close(feedback);};
    auto ok = [&](){Close(feedback, true);};

    events::void_func update_preview = [this, &feedback](){
      set_preview(m_bitmap, GetAlpha(), feedback, m_settings);
    };

    m_bitmap = feedback.GetBitmap();
    m_dialog = resizable_dialog(parent, "Alpha");

    events::on_close_window(m_dialog, cancel);

    m_alphaSlider = create_slider(raw(m_dialog),
      BoundedInt(min_t(0), get_initial_alpha(m_bitmap), max_t(255)),
      SliderDir::HORIZONTAL,
      slider_marker_BorderedLine(),
      slider_bg_Alpha(ColRGB(77,109,243)),
      m_sliderCursors,
      ui::horizontal_slider_size,
      update_preview);

    using namespace layout;
    set_sizer(m_dialog.get(), create_column({
      grow(create_row({
        {m_alphaSlider, Proportion(1), wxEXPAND}})),
         center(create_row_no_pad({make_default(m_dialog.get(),
         create_button(m_dialog.get(), "&OK", ok)),
         create_button(m_dialog.get(), "Cancel", cancel)}))}));

    set_accelerators(raw(m_dialog), {
      {key::esc, cancel}});

    center_over_parent(m_dialog);
    show(m_dialog);
    update_preview();
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
    m_settings.Update(s);
  }

  void Reinitialize(WindowFeedback& feedback) override{
    m_bitmap = feedback.GetBitmap();
    set_preview(m_bitmap, GetAlpha(), feedback, m_settings);
  }

  const Settings& GetSettings() const override{
    return m_settings;
  };

private:
  void Close(WindowFeedback& feedback, bool ok=false){
    if (m_dialog != nullptr){
      BitmapCommandPtr cmd = ok ?
        get_set_alpha_command(GetAlpha()) :
        nullptr;

      m_dialog.reset(nullptr);
      feedback.Closed(std::move(cmd));
    }
  }

  uchar GetAlpha() const{
    return static_cast<uchar>(m_alphaSlider->GetValue());
  }

  Slider* m_alphaSlider = nullptr;
  unique_dialog_ptr m_dialog;
  Settings m_settings;
  Bitmap m_bitmap;
  const SliderCursors& m_sliderCursors;
};

void show_alpha_dialog(DialogContext& c){
  c.Show(std::make_unique<AlphaDialog>(c.GetSliderCursors()));
}

} // namespace
