// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
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

namespace faint{

const wxEventType FAINT_INT_SETTING_CHANGE = wxNewEventType();
const SettingTag<IntSetting> EVT_FAINT_INT_SETTING_CHANGE(
  FAINT_INT_SETTING_CHANGE);

const wxEventType FAINT_FLOAT_SETTING_CHANGE = wxNewEventType();
const SettingTag<FloatSetting> EVT_FAINT_FLOAT_SETTING_CHANGE(
  FAINT_FLOAT_SETTING_CHANGE);

const wxEventType FAINT_BOOL_SETTING_CHANGE = wxNewEventType();
const SettingTag<BoolSetting> EVT_FAINT_BOOL_SETTING_CHANGE(
  FAINT_BOOL_SETTING_CHANGE);

const wxEventType FAINT_COLOR_SETTING_CHANGE = wxNewEventType();
const SettingTag<ColorSetting> EVT_FAINT_COLOR_SETTING_CHANGE(
  FAINT_COLOR_SETTING_CHANGE);

const wxEventType FAINT_STRING_SETTING_CHANGE = wxNewEventType();
const SettingTag<StringSetting> EVT_FAINT_STRING_SETTING_CHANGE(
  FAINT_STRING_SETTING_CHANGE);

const wxEventType SWAP_COLORS = wxNewEventType();
const wxEventTypeTag<wxCommandEvent> EVT_SWAP_COLORS(SWAP_COLORS);

SettingsEvent::SettingsEvent(const Settings& s, wxEventType type)
  : wxCommandEvent(type, -1),
    m_settings(s)
{}

const Settings& SettingsEvent::GetSettings() const{
  return m_settings;
}

const wxEventType FAINT_SETTINGS_CHANGE = wxNewEventType();
const wxEventTypeTag<SettingsEvent> EVT_FAINT_SETTINGS_CHANGE(
  FAINT_SETTINGS_CHANGE);

} // namespace
