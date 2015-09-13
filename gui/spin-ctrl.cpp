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

#include "wx/textctrl.h" // for wxEVT_TEXT_ENTER and such
#include "wx/sizer.h"
#include "wx/spinctrl.h"
#include "gui/events.hh"
#include "gui/spin-ctrl.hh"
#include "util-wx/bind-event.hh"
#include "util-wx/fwd-bind.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/layout-wx.hh"
#include "util/convenience.hh"

namespace faint{

template<typename WXCTRL_T>
class FocusRelayingCtrl : public WXCTRL_T{
public:
  FocusRelayingCtrl(wxWindow* parent) :
    WXCTRL_T(parent,
      wxID_ANY,
      wxEmptyString,
      wxDefaultPosition,
      wxSize(50, -1),
      wxSP_ARROW_KEYS | wxTE_PROCESS_ENTER)
  {
    events::on_set_focus(this, [this](){
      events::set_focus_entry(this);
    });

    events::on_kill_focus(this, [this](){
      events::kill_focus_entry(this);
    });
  }
};

using FocusRelayingSpinCtrlInt = FocusRelayingCtrl<wxSpinCtrl>;
using FocusRelayingSpinCtrlDouble = FocusRelayingCtrl<wxSpinCtrlDouble>;

static void set_value(wxSpinCtrl* ctrl, int value){
  ctrl->SetValue(value);
}

static void set_value(wxSpinCtrl* ctrl, coord value){
  ctrl->SetValue(static_cast<int>(value));
}

template<typename SettingCtrl_T>
class SizeControl : public SettingCtrl_T{
public:
  using setting_t = typename SettingCtrl_T::setting_type;
  using value_t = typename SettingCtrl_T::value_type;

  SizeControl(wxWindow* parent,
    const setting_t& setting,
    value_t value,
    const utf8_string& label)
    : SettingCtrl_T(parent, setting),
      m_changed(false),
      m_spinCtrl(nullptr)
  {
    auto sizer = new wxBoxSizer(wxVERTICAL);
    if (label.size() > 0){
      layout::add(sizer, create_label(this, label.c_str()));
    }

    // Using Int FocusRelayingCtrl regardless of SettingCtrl_T, due to
    // the wxSpinCtrlDouble sometimes ceasing to work on Windows.
    m_spinCtrl = new FocusRelayingSpinCtrlInt(this);
    set_value(m_spinCtrl, value);
    m_spinCtrl->SetRange(1, 255);
    m_spinCtrl->SetBackgroundColour(wxColour(255, 255, 255));
    sizer->Add(m_spinCtrl, 0, wxALIGN_CENTER_HORIZONTAL);
    SetSizerAndFit(sizer);

    events::on_idle(this,
      [this](){
        if (then_false(m_changed)){
          SendChangeEvent();
        }
      });

    bind_fwd(this, wxEVT_SPINCTRL,
      [this](wxSpinEvent& event){
        event.Skip();
        SendChangeEvent();
      });

    bind(this, wxEVT_TEXT_ENTER,
      [this](){
        SendChangeEvent();
      });
  }

  void SetValue(value_t value) override{
    set_value(m_spinCtrl, value);
  }

  int GetControlValue() const{
    return m_spinCtrl->GetValue();
  }

private:
  bool m_changed;
  wxSpinCtrl* m_spinCtrl;
};

class IntSizeControl : public SizeControl<IntSettingCtrl>{
public:
  using SizeControl::SizeControl;

  int GetValue() const override{
    return GetControlValue();
  }
};

IntSettingCtrl* create_int_spinner(wxWindow* parent,
  const IntSetting& setting,
  int value,
  const utf8_string& label)
{
  return new IntSizeControl(parent, setting, value, label);
}

class SemiFloatSizeControl : public SizeControl<FloatSettingControl>{
public:
  using SizeControl::SizeControl;

  coord GetValue() const override{
    return static_cast<coord>(GetControlValue());
  }
};

FloatSettingControl* create_semi_float_spinner(wxWindow* parent,
  const FloatSetting& setting,
  coord value,
  const utf8_string& label)
{
  return new SemiFloatSizeControl(parent, setting, value, label);
}

} // namespace
