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

#include <memory>
#include "wx/dialog.h"
#include "wx/notebook.h"
#include "wx/sizer.h"
#include "bitmap/paint.hh"
#include "bitmap/pattern.hh"
#include "geo/limits.hh"
#include "gui/dialog-context.hh"
#include "gui/paint-dialog.hh"
#include "gui/paint-dialog/gradient-panel.hh"
#include "gui/paint-dialog/hsl-panel.hh"
#include "gui/paint-dialog/pattern-panel.hh"
#include "text/formatting.hh"
#include "util-wx/clipboard.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/gui-util.hh"
#include "util-wx/key-codes.hh"
#include "util/optional.hh"

namespace faint{

static Optional<Color> deserialize_color(const utf8_string& str){
  wxString wxStr(to_wx(str));
  wxArrayString strs = wxSplit(to_wx(str), ',', '\0');

  size_t num = strs.size();
  if (num < 3 || 4 < num){
    return {};
  }

  std::vector<int> components;
  for (size_t i = 0; i != num; i++){
    long value;
    if (!strs[i].ToLong(&value)){
      return {};
    }

    if (can_represent<int>(value)){
      components.push_back(static_cast<int>(value));
    }
    else{
      return {};
    }
  }

  int r = components[0];
  int g = components[1];
  int b = components[2];
  int a = components.size() == 4 ? components[3] : 255;
  return valid_color(r,g,b,a) ? option(color_from_ints(r,g,b,a)) : no_option();
}

static Gradient gradient_from_color(const Color& c1, const Color& c2){
  return LinearGradient(Angle::Zero(), {{c1, 0.0}, {c2, 1.0}});
}

class PaintDialog : public wxDialog {
public:
  PaintDialog(wxWindow* parent,
    const wxString& title,
    StatusInterface& statusInfo,
    const Getter<Bitmap>& getBitmap,
    DialogContext& dialogContext)
    : wxDialog(parent, wxID_ANY, title)
  {
    m_tabs = make_wx<wxNotebook>(this, wxID_ANY,
      wxDefaultPosition, wxDefaultSize, wxNB_NOPAGETHEME);
    auto sizer = make_wx<wxBoxSizer>(wxVERTICAL);
    sizer->Add(m_tabs);

    m_panelHSL = std::make_unique<PaintPanel_HSL>(m_tabs,
      dialogContext.GetCommonCursors(),
      dialogContext.GetSliderCursors());
    m_tabs->AddPage(m_panelHSL->AsWindow(), "HSL");

    #ifdef __wxMSW__
    // Required to get the same background color in custom controls
    // (e.g. the gradient display) when the panel is in a notebook
    auto themeBg = to_faint(m_tabs->GetThemeBackgroundColour());
    #else
    // Fixme: GetThemeBackgroundColour failed on GTK.
    // (Atleast .Alpha())
    Color themeBg(255,255,255);
    #endif

    m_panelGradient = std::make_unique<PaintPanel_Gradient>(m_tabs,
      [&](const utf8_string& title, const Color& initial){
        return show_color_only_dialog(this,
          title,
          initial,
          dialogContext);
      },
      themeBg,
      statusInfo,
      dialogContext);
    m_tabs->AddPage(m_panelGradient->AsWindow(), "Gradient");

    m_panelPattern = std::make_unique<PaintPanel_Pattern>(m_tabs,
      dialogContext.GetCommonCursors(),
      getBitmap);
    m_tabs->AddPage(m_panelPattern->AsWindow(), "Pattern");

    sizer->Add(create_ok_cancel_buttons(this),
      0, wxALIGN_RIGHT|wxLEFT|wxRIGHT|wxBOTTOM, 10);
    SetSizerAndFit(sizer);
    Center(wxBOTH);

    set_accelerators(this, {
      {Alt+key::C, [=]{CopyEntries();}},
      {Alt+key::V, [=]{PasteEntries();}},
      {Ctrl+key::C, [=]{CopyPattern();}},
      {Ctrl+key::V, [=]{PastePattern();}},
      {key::Q, [=]{SelectTab(m_panelHSL->AsWindow());}},
      {key::W, [=]{SelectTab(m_panelGradient->AsWindow());}},
      {key::E, [=]{
        // Change tab on e, unless hex-field has focus
        if (m_tabs->GetCurrentPage() != m_panelHSL->AsWindow() ||
            !m_panelHSL->EatAccelerator(key::E)) {
          SelectTab(m_panelPattern->AsWindow());
        }}}
    });
  }

  Paint GetPaint() const{
    wxWindow* selected = m_tabs->GetCurrentPage();
    if (selected == m_panelHSL->AsWindow()){
      return Paint(m_panelHSL->GetColor());
    }
    else if (selected == m_panelPattern->AsWindow()){
      return Paint(m_panelPattern->GetPattern());
    }
    else if (selected == m_panelGradient->AsWindow()){
      return Paint(m_panelGradient->GetGradient());
    }
    else{
      assert(false);
      return Paint(Color(0,0,0));
    }
  }

  void SetPaint(const Paint& paint, const Color& secondary){
    visit(paint,
      [&](const Color& c){SetColor(c, secondary);},
      [&](const Pattern& p){SetPattern(p);},
      [&](const Gradient& g){SetGradient(g);});
  }

private:
  void CopyEntries(){
    if (m_tabs->GetCurrentPage() != m_panelHSL->AsWindow()){
      return;
    }
    Clipboard clipboard;
    if (!clipboard.Good()){
      return;
    }
    Color c(m_panelHSL->GetColor());
    clipboard.SetText(str_smart_rgba(c));
  }

  void CopyPattern(){
    wxWindow* selected = m_tabs->GetCurrentPage();
    if (selected == m_panelPattern->AsWindow()){
      m_panelPattern->Copy();
    }
  }


  void PastePattern(){
    wxWindow* selected = m_tabs->GetCurrentPage();
    if (selected == m_panelPattern->AsWindow()){
      m_panelPattern->Paste();
    }
  }

  void PasteEntries(){
    wxWindow* selected = m_tabs->GetCurrentPage();
    if (selected != m_panelHSL->AsWindow()){
      return;
    }

    Clipboard clip;
    if (!clip.Good()){
      return;
    }
    if (auto str = clip.GetText()){
      Optional<Color> maybeColor(deserialize_color(str.Get()));
      if (maybeColor.IsSet()){
        m_panelHSL->SetColor(maybeColor.Get());
      }
    }
  }
  void SelectTab(wxWindow* window){
    for (size_t i = 0; i != m_tabs->GetPageCount(); i++){
      if (m_tabs->GetPage(i) == window){
        m_tabs->ChangeSelection(i);
        return;
      }
    }
    assert(false);
  }

  void SetColor(const Color& color, const Color& secondary){
    m_panelHSL->SetColor(color);
    m_panelGradient->SetGradient(gradient_from_color(color, secondary));
    SelectTab(m_panelHSL->AsWindow());
  }

  void SetGradient(const Gradient& gradient){
    m_panelGradient->SetGradient(gradient);
    SelectTab(m_panelGradient->AsWindow());
  }

  void SetPattern(const Pattern& pattern){
    m_panelPattern->SetPattern(pattern);
    SelectTab(m_panelPattern->AsWindow());
  }

  std::unique_ptr<PaintPanel_Gradient> m_panelGradient;
  std::unique_ptr<PaintPanel_HSL> m_panelHSL;
  std::unique_ptr<PaintPanel_Pattern> m_panelPattern;
  wxNotebook* m_tabs;
};

static std::unique_ptr<PaintPanel_HSL> init_color_dialog_panel(wxWindow* bgPanel,
  const CommonCursors& commonCursors,
  const SliderCursors& sliderCursors,
  wxDialog* dlg)
{
  auto sizer = make_wx<wxBoxSizer>(wxVERTICAL);
  auto colorPanel = std::make_unique<PaintPanel_HSL>(bgPanel,
    commonCursors,
    sliderCursors);
  sizer->Add(colorPanel->AsWindow());

  sizer->Add(create_ok_cancel_buttons(bgPanel, dlg),
    0, wxALIGN_RIGHT|wxLEFT|wxRIGHT|wxBOTTOM, 10);
  bgPanel->SetSizer(sizer);
  return colorPanel;
}
class ColorDialog : public wxDialog {
public:
  ColorDialog(wxWindow* parent, const wxString& title,
    const CommonCursors& commonCursors,
    const SliderCursors& sliderCursors,
    const Color& initialColor)
    : wxDialog(parent, wxID_ANY, title)
  {
    auto sizer = make_wx<wxBoxSizer>(wxVERTICAL);
    auto bg = create_panel(this);
    sizer->Add(bg);
    m_panelHSL = init_color_dialog_panel(bg, commonCursors, sliderCursors, this);
    m_panelHSL->SetColor(initialColor);
    SetSizerAndFit(sizer);
    Center(wxBOTH);
  }

  Color GetColor() const{
    return m_panelHSL->GetColor();
  }
private:
  std::unique_ptr<PaintPanel_HSL> m_panelHSL;
};

Optional<Color> show_color_only_dialog(wxWindow* parent,
  const utf8_string& title,
  const Color& initial,
  DialogContext& context)
{
  ColorDialog dlg(parent, to_wx(title),
    context.GetCommonCursors(),
    context.GetSliderCursors(),
    initial);
  return context.ShowModal(dlg) == DialogChoice::OK ?
    option(dlg.GetColor()) : no_option();
}

Optional<Paint> show_paint_dialog(wxWindow* parent,
  const utf8_string& title,
  const Paint& initial,
  const Color& secondary,
  const Getter<Bitmap>& getBitmap,
  StatusInterface& statusInfo,
  DialogContext& context)
{
  PaintDialog dlg(parent, to_wx(title), statusInfo, getBitmap, context);
  dlg.SetPaint(initial, secondary);
  return context.ShowModal(dlg) == DialogChoice::OK ?
    option(dlg.GetPaint()) :
    no_option();
}

} // namespace
