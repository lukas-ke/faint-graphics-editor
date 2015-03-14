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

#ifndef FAINT_SETTING_EVENTS_HH
#define FAINT_SETTING_EVENTS_HH
#include "wx/event.h"
#include "util/settings.hh"

namespace faint{

template<typename T>
class SettingEvent : public wxCommandEvent{
public:
  SettingEvent(const T& s, const typename T::ValueType& v, wxEventType type)
    : wxCommandEvent(type, -1),
      m_setting(s),
      m_value(v)
  {}

  T GetSetting() const{
    return m_setting;
  }

  typename T::ValueType GetValue() const{
    return m_value;
  }
private:
  T m_setting;
  typename T::ValueType m_value;
};

template<typename T>
using SettingTag = wxEventTypeTag<SettingEvent<T>>;

extern const wxEventType FAINT_INT_SETTING_CHANGE;
extern const SettingTag<IntSetting> EVT_FAINT_INT_SETTING_CHANGE;

extern const wxEventType FAINT_FLOAT_SETTING_CHANGE;
extern const SettingTag<FloatSetting> EVT_FAINT_FLOAT_SETTING_CHANGE;

extern const wxEventType FAINT_BOOL_SETTING_CHANGE;
extern const SettingTag<BoolSetting> EVT_FAINT_BOOL_SETTING_CHANGE;

extern const wxEventType FAINT_COLOR_SETTING_CHANGE;
extern const SettingTag<ColorSetting> EVT_FAINT_COLOR_SETTING_CHANGE;

extern const wxEventType FAINT_STRING_SETTING_CHANGE;
extern const SettingTag<StringSetting> EVT_FAINT_STRING_SETTING_CHANGE;

extern const wxEventType SWAP_COLORS;
extern const wxEventTypeTag<wxCommandEvent> EVT_SWAP_COLORS;

class SettingsEvent : public wxCommandEvent{
public:
  SettingsEvent(const Settings&, wxEventType);
  const Settings& GetSettings() const;
private:
  Settings m_settings;
};

extern const wxEventType FAINT_SETTINGS_CHANGE;
extern const wxEventTypeTag<SettingsEvent> EVT_FAINT_SETTINGS_CHANGE;

} // namespace

#endif
