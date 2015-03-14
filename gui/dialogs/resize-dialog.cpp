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

#include "wx/button.h" // Fixme
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/paint.hh"
#include "geo/geo-func.hh"
#include "geo/int-point.hh"
#include "geo/int-rect.hh"
#include "geo/int-size.hh"
#include "geo/rect.hh"
#include "gui/art-container.hh"
#include "gui/command-window.hh"
#include "gui/layout.hh"
#include "gui/math-text-ctrl.hh"
#include "gui/resize-dialog.hh"
#include "gui/with-label.hh"
#include "util-wx/clipboard-util.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/fwd-bind.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/gui-util.hh"
#include "util-wx/key-codes.hh"
#include "util-wx/bind-event.hh"
#include "util-wx/layout-wx.hh"
#include "util/color-bitmap-util.hh"
#include "util/setting-id.hh"

namespace faint{

// Helper for proportional update
static coord get_change_ratio(const MathTextCtrl& ctrl){
  return ctrl.GetValue() / ctrl.GetOldValue();
}

// Helper for proportional update
static void set_from_ratio(MathTextCtrl& ctrl, coord ratio){
  ctrl.SetValue(ctrl.GetOldValue() * ratio);
}

template<typename FUNC>
static WithLabel<wxButton>* defaultable_button(wxWindow* parent,
  const wxString& toolTip,
  FUNC&& f)
{
  auto* withLabel = label_below<wxButton>(parent, "(Default)", wxID_ANY, "",
    wxDefaultPosition, to_wx(big_button_size));
  auto& b = withLabel->GetWindow();
  bind(&b, wxEVT_BUTTON, std::move(f));
  b.SetToolTip(toolTip);
  return withLabel;
}

static IntRect centered_resize_rect(const IntSize& newSize,
  const IntSize& oldSize)
{
  return IntRect(IntPoint(-(newSize.w - oldSize.w) / 2,
      -(newSize.h - oldSize.h) / 2),
    newSize);
}

class ResizeDialog : public CommandWindow{
private:
  enum class FocusMode{CONSIDER_FOCUS, IGNORE_FOCUS};

public:
  ResizeDialog(const ArtContainer& art,
    std::unique_ptr<ResizeDialogContext> ctx)
    : m_art(art),
      m_ctx(std::move(ctx)),
      m_dialog(null_dialog())
  {
    m_settings.Set(ts_Bg, m_ctx->GetBackground());
  }

  bool MouseMove(const PosInfo&) override{
    return false;
  }

  bool MouseDown(const PosInfo&) override{
    return false;
  }

  void Draw(FaintDC&, Overlays&, const PosInfo&) override{}

  void Show(wxWindow& parent, WindowFeedback& feedback) override{
    auto cancel = [&](){Close(feedback);};

    auto rescale = [&](){
      SetDefaultOptions(ResizeDialogOptions::RESCALE);
      UpdateProportions(FocusMode::CONSIDER_FOCUS);

      auto newSize = GetSize();
      if (newSize != GetOldSize() && reasonable_bitmap_size(newSize)){
        m_ctx->Rescale(newSize, GetScaleQuality());
      }
      Close(feedback);
    };

    auto resize_centered = [&](){
      SetDefaultOptions(ResizeDialogOptions::RESIZE_CENTER);
      UpdateProportions(FocusMode::CONSIDER_FOCUS);

      const auto oldSize = GetOldIntSize();
      const auto newSize = GetIntSize();
      if (oldSize != newSize && reasonable_bitmap_size(newSize)){
        m_ctx->Resize(centered_resize_rect(newSize, oldSize),
          m_settings.Get(ts_Bg));
      }

      Close(feedback);
    };

    auto resize_top_left = [&](){
      SetDefaultOptions(ResizeDialogOptions::RESIZE_TOP_LEFT);
      UpdateProportions(FocusMode::CONSIDER_FOCUS);

      IntSize newSize(GetIntSize());
      if (newSize != GetOldIntSize() && reasonable_bitmap_size(newSize)){
        m_ctx->Resize(rect_from_size(newSize), m_settings.Get(ts_Bg));
      }

      Close(feedback);
    };

    m_dialog = create_dialog(parent, m_ctx->GetTitle().c_str());
    events::on_close_window(m_dialog, cancel);

    const auto& options(m_ctx->GetOptions());
    m_proportional = create_checkbox(m_dialog.get(), "&Proportional",
      options.proportional,
      [&](){UpdateProportions(FocusMode::CONSIDER_FOCUS);});

    if (m_ctx->AllowNearestNeighbour()){
      m_nearest = create_checkbox(m_dialog.get(), "&Nearest neighbour",
        options.nearest);
    }

    if (m_ctx->AllowResize()){
      m_resizeTopLeft = defaultable_button(raw(m_dialog),
        "Resize drawing area right and down (Key: Q)",
        resize_top_left);

      m_resizeCentered = defaultable_button(raw(m_dialog),
        "Resize drawing area in all directions (Key: W)",
        resize_centered);
    }

    m_rescale = defaultable_button(raw(m_dialog), "Scale the image (Key: E)",
      rescale);

    if (!m_ctx->AllowResize()){
      // There's no reason to indicate the default when only one
      // option is available
      m_rescale->HideLabel();
    }

    auto cancelButton = create_button(m_dialog.get(), "Cancel", big_button_size,
      cancel);

    auto size(m_ctx->GetSize().Visit(
      [](const IntSize& sz){
        return floated(sz);
      },
      [](const Size& sz){
        return sz;
      }));

    using namespace layout;
    set_sizer(m_dialog.get(), create_column({
      create_row({raw(m_proportional), raw(m_nearest)}),

      create_row({
        m_width = focused(label_left<MathTextCtrl>(raw(m_dialog), "&Width", size.w,
          [&](){
            m_lastChanged = m_width;
            UpdateProportions(FocusMode::IGNORE_FOCUS);
          })),

        m_height = label_left<MathTextCtrl>(raw(m_dialog), "&Height", size.h,
            [&](){
              m_lastChanged = m_height;
              UpdateProportions(FocusMode::IGNORE_FOCUS);
            })}),

      create_row_outer_pad({raw(m_resizeTopLeft),
        raw(m_resizeCentered),
        raw(m_rescale),
        {raw(cancelButton), 0, wxALIGN_TOP}})}));

    m_width->GetWindow().FitSizeTo("1024 (100%)");
    m_height->GetWindow().FitSizeTo("1024 (100%)");

    SetDefaultResizeOption(options.defaultButton);
    UpdateBitmaps();

    set_accelerators(raw(m_dialog), {
      // Since P, N are unused for text in the dialog, allow using
      // without Alt-modifier. Also redundantly add Alt+<key> as
      // accelerators, to prevent the default behavior of focus
      // changing to the check-boxes from the entry fields
      {key::P, Alt+key::P, [&](){toggle(m_proportional);}},
      cond(m_nearest != nullptr,
        key::N, Alt+key::N, [&](){toggle(m_nearest);}),

      // Each resize option
      cond(m_resizeTopLeft != nullptr,
         key::Q, resize_top_left),

      cond(m_resizeCentered != nullptr,
        key::W, resize_centered),

      {key::E, rescale},

      {key::esc, cancel},

      // Alt+C copies both the width and height
      {Alt+key::C,
       [&](){
         clipboard_copy_size({m_width->GetWindow().GetValue(),
           m_height->GetWindow().GetValue()});
        }},

      // Alt+V pastes comma separated values from the clipboard
      // to the width and height.
      {Alt+key::V,
        [&](){
          clipboard_get_size().Visit(
            [this](const Size& size){
              // Disable proportional resize, since both pasted values
              // should remain intact
              clear(m_proportional);
              SetSize(size);
            });}}
    });

    center_over_parent(m_dialog.get());
    show(m_dialog.get());
  }

  bool IsShown() const override{
    return m_dialog != nullptr;
  }

  void UpdateSettings(const Settings& s) override{
    m_settings.Update(s);
    if (m_dialog != nullptr){
      UpdateBitmaps();
    }
    set_focus(m_dialog.get());
  }

  void Reinitialize(WindowFeedback&) override{
    // Fixme: Need to get the new size, new targets and so on. ;_;
  }

  const Settings& GetSettings() const override{
    return m_settings;
  }

private:
  void UpdateBitmaps(){
    const auto scaleBitmap(to_faint(m_art.Get(Icon::RESIZEDLG_SCALE)));
    const IntSize bitmapSize(scaleBitmap.GetSize());

    if (m_resizeTopLeft != nullptr){
      set_bitmap(*m_resizeTopLeft,
        resize_bitmap(m_settings.Get(ts_Bg),
          bitmapSize,
          to_faint(m_art.Get(Icon::RESIZEDLG_PLACEMENT)),
          IntPoint(0,0)));
    }

    if (m_resizeCentered != nullptr){
      const wxBitmap& placement(m_art.Get(Icon::RESIZEDLG_PLACEMENT));
      IntPoint topLeft = centered(to_faint(placement.GetSize()),
        bitmapSize).TopLeft();

      set_bitmap(*m_resizeCentered,
        resize_bitmap(m_settings.Get(ts_Bg),
          bitmapSize,
          to_faint(m_art.Get(Icon::RESIZEDLG_PLACEMENT)),
          topLeft));
    }

    set_bitmap(*m_rescale, scaleBitmap);
  }

  void Close(WindowFeedback& feedback){
    m_dialog = nullptr;
    feedback.Closed();
  }

  void SetDefaultResizeOption(ResizeDialogOptions::ResizeType type){
    if (m_resizeTopLeft != nullptr){
      m_resizeTopLeft->HideLabel();
    }
    if (m_resizeCentered != nullptr){
      m_resizeCentered->HideLabel();
    }
    m_rescale->HideLabel();

    switch(type){
    case ResizeDialogOptions::RESCALE:
      layout::make_default(m_dialog.get(), &m_rescale->GetWindow());
      m_rescale->ShowLabel();
      break;

    case ResizeDialogOptions::RESIZE_CENTER:
      if (m_resizeCentered != nullptr){
        layout::make_default(m_dialog.get(), &m_resizeCentered->GetWindow());
        m_resizeCentered->ShowLabel();
      }
      break;

    case ResizeDialogOptions::RESIZE_TOP_LEFT:
      if (m_resizeTopLeft != nullptr){
        layout::make_default(m_dialog.get(), &m_resizeTopLeft->GetWindow());
        m_resizeTopLeft->ShowLabel();
      }
      break;
    }
  }

  auto GetProportionMasterSlave(FocusMode f){
    if (f == FocusMode::CONSIDER_FOCUS){
      if (m_width->GetWindow().HasFocus()){
        return std::make_pair(m_width, m_height);
      }
      else if (m_height->GetWindow().HasFocus()){
        return std::make_pair(m_height, m_width);
      }
    }

    if (m_lastChanged == nullptr || m_lastChanged == m_width){
      return std::make_pair(m_width, m_height);
    }
    else {
      return std::make_pair(m_height, m_width);
    }
  }

  void UpdateProportions(FocusMode f){
    if (not_set(m_proportional)){
      return;
    }

    // Update the width or height field proportionally relative to its
    // old value, using the last edited or focused edit field to
    // determine the factor.


    auto p = GetProportionMasterSlave(f);
    set_from_ratio(p.second->GetWindow(),
      get_change_ratio(p.first->GetWindow()));
  }

  void SetDefaultOptions(ResizeDialogOptions::ResizeType type){
    auto options(m_ctx->GetOptions());
    if (m_nearest != nullptr){
      options.nearest = get(m_nearest);
    }
    options.proportional = get(m_proportional);

    if (m_ctx->AllowResize()){
      // Don't change the default action when only scaling was available.
      options.defaultButton = type;
    }
    m_ctx->SetDefaultOptions(options);
  }

  IntSize GetIntSize(){
    return rounded(GetSize());
  }

  Size GetSize(){
    return {m_width->GetWindow().GetValue(), m_height->GetWindow().GetValue()};
  }

  IntSize GetOldIntSize() const{
    return rounded(GetOldSize());
  }

  Size GetOldSize() const{
    return m_ctx->GetSize().Visit(
      [](const IntSize& sz){return floated(sz);},
      [](const Size& sz){return sz;});
  }

  void SetSize(const Size& sz){
    m_width->GetWindow().SetValue(sz.w);
    m_height->GetWindow().SetValue(sz.h);
  }

  ScaleQuality GetScaleQuality() const{
    return (m_nearest != nullptr && get(m_nearest)) ?
      ScaleQuality::NEAREST :
      ScaleQuality::BILINEAR;
  }

  const ArtContainer& m_art;
  std::unique_ptr<ResizeDialogContext> m_ctx;
  unique_dialog_ptr m_dialog;
  Settings m_settings;
  WithLabel<wxButton>* m_resizeTopLeft = nullptr;
  WithLabel<wxButton>* m_resizeCentered = nullptr;
  WithLabel<wxButton>* m_rescale = nullptr;
  WithLabel<MathTextCtrl>* m_width = nullptr;
  WithLabel<MathTextCtrl>* m_height = nullptr;
  decltype(m_width) m_lastChanged = nullptr;
  wxCheckBox* m_proportional = nullptr;
  wxCheckBox* m_nearest = nullptr;
};

void show_resize_dialog(DialogContext& c,
  const ArtContainer& art,
  std::unique_ptr<ResizeDialogContext> rdc)
{
  c.Show(std::make_unique<ResizeDialog>(art, std::move(rdc)));
}

} // namespace
