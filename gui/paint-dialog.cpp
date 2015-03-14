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

#include <functional>
#include <memory>
#include "wx/dialog.h"
#include "wx/notebook.h"
#include "wx/sizer.h"
#include "bitmap/paint.hh"
#include "bitmap/pattern.hh"
#include "gui/paint-dialog.hh"
#include "gui/paint-dialog/gradient-panel.hh"
#include "gui/paint-dialog/hsl-panel.hh"
#include "gui/paint-dialog/pattern-panel.hh"
#include "text/formatting.hh"
#include "util/optional.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/clipboard.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/gui-util.hh"
#include "util-wx/key-codes.hh"

namespace faint{

static Optional<Color> deserialize_color(const utf8_string& str){
  wxString wxStr(to_wx(str));
  wxArrayString strs = wxSplit(to_wx(str), ',', '\0');

  size_t num = strs.size();
  if (num < 3 || 4 < num){
    return no_option();
  }

  std::vector<int> components;
  for (size_t i = 0; i != strs.size(); i++){
    long value;
    if (!strs[i].ToLong(&value)){
      return no_option();
    }
    components.push_back(static_cast<int>(value)); // Fixme: Check cast
  }

  int r = components[0];
  int g = components[1];
  int b = components[2];
  int a = components.size() == 4 ? components[3] : 255;
  return valid_color(r,g,b,a) ? option(color_from_ints(r,g,b,a)) : no_option();
}

static Gradient gradient_from_color(const Color& c){
  LinearGradient g;
  g.Add(ColorStop(c, 0.0));
  g.Add(ColorStop(color_white, 1.0));
  return Gradient(g);
}

class PaintDialog : public wxDialog {
public:
  PaintDialog(wxWindow* parent,
    const wxString& title,
    StatusInterface& statusInfo,
    DialogContext& dialogContext)
    : wxDialog(parent, wxID_ANY, title)
  {
    m_tabs = new wxNotebook(this, wxID_ANY,
      wxDefaultPosition, wxDefaultSize, wxNB_NOPAGETHEME);
    wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(m_tabs);

    m_panelHSL = std::make_unique<PaintPanel_HSL>(m_tabs);
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
      themeBg,
      statusInfo,
      dialogContext);
    m_tabs->AddPage(m_panelGradient->AsWindow(), "Gradient");

    m_panelPattern = std::make_unique<PaintPanel_Pattern>(m_tabs);
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
      {key::E, [=]{SelectTab(m_panelPattern->AsWindow());}}
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
    else {
      assert(false);
      return Paint(Color(0,0,0));
    }
  }

  void SetPaint(const Paint& paint){
    using namespace std::placeholders;
    visit(paint,
      std::bind(&PaintDialog::SetColor, this, _1),
      std::bind(&PaintDialog::SetPattern, this, _1),
      std::bind(&PaintDialog::SetGradient, this, _1));
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

  void SetColor(const Color& color){
    m_panelHSL->SetColor(color);
    m_panelGradient->SetGradient(gradient_from_color(color));
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
  wxDialog* dlg)
{
  wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
  auto colorPanel = std::make_unique<PaintPanel_HSL>(bgPanel);
  sizer->Add(colorPanel->AsWindow());

  sizer->Add(create_ok_cancel_buttons(bgPanel, dlg),
    0, wxALIGN_RIGHT|wxLEFT|wxRIGHT|wxBOTTOM, 10);
  bgPanel->SetSizer(sizer);
  return colorPanel;
}
class ColorDialog : public wxDialog {
public:
  ColorDialog(wxWindow* parent, const wxString& title, const Color& initialColor)
    : wxDialog(parent, wxID_ANY, title)
  {
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    auto bg = create_panel(this);
    sizer->Add(bg);
    m_panelHSL = init_color_dialog_panel(bg, this);
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
  const wxString& title,
  const Color& initial,
  DialogContext& context)
{
  ColorDialog dlg(parent, title, initial);
  int result = context.ShowModal(dlg);
  if (result == wxID_OK){
    return option(dlg.GetColor());
  }
  return no_option();
}

Optional<Paint> show_paint_dialog(wxWindow* parent,
  const wxString& title,
  const Paint& initial,
  StatusInterface& statusInfo,
  DialogContext& context)
{
  PaintDialog dlg(parent, title, statusInfo, context);
  dlg.SetPaint(initial);
  int result = context.ShowModal(dlg);
  if (result == wxID_OK){
    return option(dlg.GetPaint());
  }
  return no_option();
}

} // namespace
