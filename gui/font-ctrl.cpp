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

#include "wx/fontdlg.h"
#include "wx/sizer.h"
#include "gui/dialog-context.hh"
#include "gui/font-ctrl.hh"
#include "gui/setting-events.hh"
#include "gui/tool-setting-ctrl.hh"
#include "util/setting-id.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/gui-util.hh"

namespace faint{

static Settings settings_from_font(const wxFont& f){
  Settings s;
  s.Set(ts_FontFace, to_faint(f.GetFaceName()));
  s.Set(ts_FontSize, f.GetPointSize());
  s.Set(ts_FontBold, f.GetWeight() == wxFONTWEIGHT_BOLD);
  s.Set(ts_FontItalic, f.GetStyle() == wxFONTSTYLE_ITALIC);
  return s;
}

static wxFont font_from_settings(const Settings& s){
  return wxFont(s.Get(ts_FontSize),
    wxFONTFAMILY_DEFAULT, // Font family is ignored when specifying a face name
    s.Get(ts_FontItalic) ? wxFONTSTYLE_ITALIC : wxFONTSTYLE_NORMAL,
    s.Get(ts_FontBold) ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL,
    false, // Underline not supported
    to_wx(s.Get(ts_FontFace)));
}

class FontCtrl : public ToolSettingCtrl {
public:
  FontCtrl(wxWindow* parent, DialogContext& dialogContext)
    : ToolSettingCtrl(parent),
      m_dialogContext(dialogContext),
      m_settings(settings_from_font(wxFont(wxFontInfo())))
  {
    wxBoxSizer* vSizer = new wxBoxSizer(wxVERTICAL);
    wxBoxSizer* hSizer = new wxBoxSizer(wxHORIZONTAL);
    m_button = create_button(this, "Font", Width(50),
      [this](){
        wxFontData initial;
        initial.SetInitialFont(font_from_settings(m_settings));
        initial.EnableEffects(false);
        wxFontDialog dlg(GetParent(), initial);

        if (m_dialogContext.ShowModal(dlg) == wxID_OK){
          m_settings = settings_from_font(dlg.GetFontData().GetChosenFont());
          SendChangeEvent();
        }
      });

    hSizer->Add(raw(m_button));
    vSizer->Add(hSizer);
    SetSizerAndFit(vSizer);

  }

  bool UpdateControl(const Settings& s) override{
    m_settings.Update(s);
    if (s.Has(ts_FontFace)){
      set_tooltip(m_button, s.Get(ts_FontFace));
      return true;
    }
    return false;
  }

  void SendChangeEvent() override{
    SettingsEvent event(m_settings, FAINT_SETTINGS_CHANGE);
    event.SetEventObject(this);
    GetEventHandler()->ProcessEvent(event);
  }

private:
  wxButton* m_button;
  DialogContext& m_dialogContext;
  Settings m_settings;
};

ToolSettingCtrl* create_font_ctrl(wxWindow* parent, DialogContext& dialogContext){
  return new FontCtrl(parent, dialogContext);
}

} // namespace
