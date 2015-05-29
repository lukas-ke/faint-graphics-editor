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

const wxEventType FAINT_IntSettingChange = wxNewEventType();
const SettingTag<IntSetting> EVT_FAINT_IntSettingChange(
  FAINT_IntSettingChange);

const wxEventType FAINT_FloatSettingChange = wxNewEventType();
const SettingTag<FloatSetting> EVT_FAINT_FloatSettingChange(
  FAINT_FloatSettingChange);

const wxEventType FAINT_BoolSettingChange = wxNewEventType();
const SettingTag<BoolSetting> EVT_FAINT_BoolSettingChange(
  FAINT_BoolSettingChange);

const wxEventType FAINT_PaintSettingChange = wxNewEventType();
const SettingTag<PaintSetting> EVT_FAINT_PaintSettingChange(
  FAINT_PaintSettingChange);

const wxEventType FAINT_StringSettingChange = wxNewEventType();
const SettingTag<StringSetting> EVT_FAINT_StringSettingChange(
  FAINT_StringSettingChange);

const wxEventType FAINT_SwapColors = wxNewEventType();
const wxEventTypeTag<wxCommandEvent> EVT_FAINT_SwapColors(FAINT_SwapColors);

SettingsEvent::SettingsEvent(const Settings& s, wxEventType type)
  : wxCommandEvent(type, -1),
    m_settings(s)
{}

const Settings& SettingsEvent::GetSettings() const{
  return m_settings;
}

const wxEventType FAINT_SettingsChange = wxNewEventType();
const wxEventTypeTag<SettingsEvent> EVT_FAINT_SettingsChange(
  FAINT_SettingsChange);

} // namespace
