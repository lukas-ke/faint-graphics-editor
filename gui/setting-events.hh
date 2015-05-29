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

extern const wxEventType FAINT_IntSettingChange;
extern const SettingTag<IntSetting> EVT_FAINT_IntSettingChange;

extern const wxEventType FAINT_FloatSettingChange;
extern const SettingTag<FloatSetting> EVT_FAINT_FloatSettingChange;

extern const wxEventType FAINT_BoolSettingChange;
extern const SettingTag<BoolSetting> EVT_FAINT_BoolSettingChange;

extern const wxEventType FAINT_PaintSettingChange;
extern const SettingTag<PaintSetting> EVT_FAINT_PaintSettingChange;

extern const wxEventType FAINT_StringSettingChange;
extern const SettingTag<StringSetting> EVT_FAINT_StringSettingChange;

extern const wxEventType FAINT_SwapColors;
extern const wxEventTypeTag<wxCommandEvent> EVT_FAINT_SwapColors;

class SettingsEvent : public wxCommandEvent{
public:
  SettingsEvent(const Settings&, wxEventType);
  const Settings& GetSettings() const;
private:
  Settings m_settings;
};

extern const wxEventType FAINT_SettingsChange;
extern const wxEventTypeTag<SettingsEvent> EVT_FAINT_SettingsChange;

} // namespace

#endif
