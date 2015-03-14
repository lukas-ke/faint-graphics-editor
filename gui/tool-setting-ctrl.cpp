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

#include "gui/setting-events.hh"
#include "gui/tool-setting-ctrl.hh"

namespace faint{

ToolSettingCtrl::ToolSettingCtrl(wxWindow* parent)
  : wxPanel(parent)
{}

IntSettingCtrl::IntSettingCtrl(wxWindow* parent, const IntSetting& s)
  : ToolSettingCtrl(parent),
    m_setting(s)
{}

IntSetting IntSettingCtrl::GetSetting() const{
  return m_setting;
}

bool IntSettingCtrl::UpdateControl(const Settings& s){
  if (s.Has(m_setting)){
    SetValue(s.Get(m_setting));
    return true;
  }
  return false;
}

void IntSettingCtrl::SendChangeEvent(){
  SettingEvent<IntSetting> event(m_setting, GetValue(),
    FAINT_INT_SETTING_CHANGE);
  event.SetEventObject(this);
  GetEventHandler()->ProcessEvent(event);
}

FloatSettingControl::FloatSettingControl(wxWindow* parent, const FloatSetting& s)
  : ToolSettingCtrl(parent),
  m_setting(s)
{}

FloatSetting FloatSettingControl::GetSetting() const{
  return m_setting;
}

bool FloatSettingControl::UpdateControl(const Settings& s){
  if (s.Has(m_setting)){
    SetValue(s.Get(m_setting));
    return true;
  }
  return false;
}

void FloatSettingControl::SendChangeEvent(){
  SettingEvent<FloatSetting> event(m_setting, GetValue(),
    FAINT_FLOAT_SETTING_CHANGE);
  event.SetEventObject(this);
  GetEventHandler()->ProcessEvent(event);
}

BoolSettingControl::BoolSettingControl(wxWindow* parent, const BoolSetting& s)
  : ToolSettingCtrl(parent),
    m_setting(s)
{}

BoolSetting BoolSettingControl::GetSetting() const{
  return m_setting;
}

void BoolSettingControl::SendChangeEvent(){
  SettingEvent<BoolSetting> event(m_setting, GetValue(),
    FAINT_BOOL_SETTING_CHANGE);
  event.SetEventObject(this);
  GetEventHandler()->ProcessEvent(event);
}

bool BoolSettingControl::UpdateControl(const Settings& s){
  if (s.Has(m_setting)){
    SetValue(s.Get(m_setting));
    return true;
  }
  return false;
}


StringSettingControl::StringSettingControl(wxWindow* parent,
  const StringSetting& s)
  : ToolSettingCtrl(parent),
    m_setting(s)
{}

StringSetting StringSettingControl::GetSetting() const{
  return m_setting;
}

void StringSettingControl::SendChangeEvent(){
  SettingEvent<StringSetting> event(m_setting, GetValue(),
    FAINT_STRING_SETTING_CHANGE);
  event.SetEventObject(this);
  GetEventHandler()->ProcessEvent(event);
}

bool StringSettingControl::UpdateControl(const Settings& s){
  if (s.Has(m_setting)){
    SetValue(s.Get(m_setting));
    return true;
  }
  return false;
}

} // namespace
