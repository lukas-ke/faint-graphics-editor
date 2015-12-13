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

#include <cassert>
#include "util/bound-setting.hh"
#include "util/settings.hh"

namespace faint{

setting_id new_setting_id(){
  static int max_id = 1003;
  return max_id++;
}

Settings::Settings(){
}

// Implement all Set-variants
#define DEFINE_SETTER(SettingType)\
template<>\
void Settings::Set(const SettingType& s,\
const SettingType::ValueType& v)\
{\
    m_##SettingType##Map[s] =  v;\
}
// ---

DEFINE_SETTER(BoolSetting)
DEFINE_SETTER(PaintSetting)
DEFINE_SETTER(IntSetting)
DEFINE_SETTER(StringSetting)
DEFINE_SETTER(FloatSetting)

// Implement all Get-variants
#define DEFINE_GETTER(SettingType)\
template<>\
const SettingType::ValueType& Settings::Get(const SettingType& s) const { \
  auto it = m_##SettingType##Map.find(s); \
  assert(it != m_##SettingType##Map.end()); \
  return it->second; \
}
// ---

DEFINE_GETTER(BoolSetting)
DEFINE_GETTER(PaintSetting)
DEFINE_GETTER(IntSetting)
DEFINE_GETTER(StringSetting)
DEFINE_GETTER(FloatSetting)

// Implement Get-variants which returns a default value for missing settings
#define DEFINE_GETTER_DEFAULT(SettingType)\
template<> const SettingType::ValueType& Settings::GetDefault(const SettingType& s, const SettingType::ValueType& defaultValue) const { \
  auto it = m_##SettingType##Map.find(s); \
  return (it == m_##SettingType##Map.end()) ? defaultValue : it->second; \
}
// ---

DEFINE_GETTER_DEFAULT(BoolSetting)
DEFINE_GETTER_DEFAULT(PaintSetting)
DEFINE_GETTER_DEFAULT(IntSetting)
DEFINE_GETTER_DEFAULT(StringSetting)
DEFINE_GETTER_DEFAULT(FloatSetting)

template<>
bool Settings::Has(const IntSetting& setting) const{
  return (m_IntSettingMap.find(setting) != m_IntSettingMap.end());
}

template<>
bool Settings::Has(const StringSetting& setting) const{
  return (m_StringSettingMap.find(setting) != m_StringSettingMap.end());
}

template<>
bool Settings::Has(const BoolSetting& setting) const{
  return (m_BoolSettingMap.find(setting) != m_BoolSettingMap.end());
}

template<>
bool Settings::Has(const PaintSetting& setting) const{
  return (m_PaintSettingMap.find(setting) != m_PaintSettingMap.end());
}

template<>
bool Settings::Has(const FloatSetting& setting) const{
  return (m_FloatSettingMap.find(setting) != m_FloatSettingMap.end());
}

template<>
void Settings::Erase(const IntSetting& setting){
  m_IntSettingMap.erase(setting);
}

template<>
void Settings::Erase(const StringSetting& setting){
  m_StringSettingMap.erase(setting);
}

template<>
void Settings::Erase(const BoolSetting& setting){
  m_BoolSettingMap.erase(setting);
}

template<>
void Settings::Erase(const FloatSetting& setting){
  m_FloatSettingMap.erase(setting);
}

template<>
void Settings::Erase(const PaintSetting& setting){
  m_PaintSettingMap.erase(setting);
}

bool Settings::Has(const UntypedSetting& s) const{
  return Has(IntSetting(s.ToInt())) ||
    Has(StringSetting(s.ToInt())) ||
    Has(BoolSetting(s.ToInt())) ||
    Has(PaintSetting(s.ToInt())) ||
    Has(FloatSetting(s.ToInt()));
}

template<typename T>
bool update_map_values(T& targetMap, const Settings& source){
  // Updatate the values of all settings that exist in both targetMap
  // and source with the values from source.
  bool updated = false;
  for (auto& targetItem : targetMap){
    const auto& settingId = targetItem.first;
    if (source.Has(settingId)){
      const auto& v = source.Get(settingId);
      if (targetItem.second != v){
        targetItem.second = v;
        updated = true;
      }
    }
  }
  return updated;
}

bool Settings::Update(const Settings& other){
  return update_map_values(m_BoolSettingMap, other) |
    update_map_values(m_PaintSettingMap, other) |
    update_map_values(m_FloatSettingMap, other) |
    update_map_values(m_IntSettingMap, other) |
    update_map_values(m_StringSettingMap, other);
}

bool Settings::Empty() const{
  return m_BoolSettingMap.empty() &&
    m_PaintSettingMap.empty() &&
    m_FloatSettingMap.empty() &&
    m_IntSettingMap.empty() &&
    m_StringSettingMap.empty();
}

template<typename T>
bool update_map(const Setting<T>& s,
  const typename Setting<T>::ValueType& v,
  std::map<Setting<T>,
  typename Setting<T>::ValueType>& map)
{
  auto iter = map.find(s);
  if (iter != end(map) && iter->second != v){
    iter->second = v;
    return true;
  }
  return false;
}

bool Settings::Update(const BoundSetting& setting){
  return setting.Visit(
   [&](BoolSetting s, BoolSetting::ValueType v) -> bool{
     return update_map(s, v, m_BoolSettingMap);
    },
    [&](IntSetting s, IntSetting::ValueType v) -> bool{
      return update_map(s, v, m_IntSettingMap);
    },
    [&](StringSetting s, StringSetting::ValueType v) -> bool{
      return update_map(s, v, m_StringSettingMap);
    },
    [&](FloatSetting s, FloatSetting::ValueType v) -> bool{
      return update_map(s, v, m_FloatSettingMap);
    },
    [&](PaintSetting s, PaintSetting::ValueType v) -> bool{
      return update_map(s, v, m_PaintSettingMap);
    });
}


template<typename T>
void append_as_untyped(std::vector<UntypedSetting>& untypedIds, const T& map){
  for (auto& item : map){
    untypedIds.push_back(UntypedSetting(item.first));
  }
}

std::vector<UntypedSetting> Settings::GetRaw() const{
  std::vector<UntypedSetting> untyped;
  append_as_untyped(untyped, m_BoolSettingMap);
  append_as_untyped(untyped, m_IntSettingMap);
  append_as_untyped(untyped, m_StringSettingMap);
  append_as_untyped(untyped, m_PaintSettingMap);
  append_as_untyped(untyped, m_FloatSettingMap);
  return untyped;
}

template<typename T>
void update_map_content(T& targetMap, const T& otherMap){
  // Set the value of all settings from otherMap in targetMap (add the
  // settings if they do not exist).
  for (const auto& item : otherMap){
    targetMap[item.first] = item.second;
  }
}

void Settings::UpdateAll(const Settings& other){
  update_map_content(m_BoolSettingMap, other.m_BoolSettingMap);
  update_map_content(m_PaintSettingMap, other.m_PaintSettingMap);
  update_map_content(m_FloatSettingMap, other.m_FloatSettingMap);
  update_map_content(m_IntSettingMap, other.m_IntSettingMap);
  update_map_content(m_StringSettingMap, other.m_StringSettingMap);
}

void Settings::Clear(){
  m_BoolSettingMap.clear();
  m_PaintSettingMap.clear();
  m_FloatSettingMap.clear();
  m_IntSettingMap.clear();
  m_StringSettingMap.clear();
}

BoolSetting::ValueType Settings::Not(const BoolSetting& s) const{
  auto it = m_BoolSettingMap.find(s);
  assert(it != m_BoolSettingMap.end());
  return !(it->second);
}

BoundSetting::BoundSetting(const BoolSetting s, BoolSetting::ValueType v){
  m_boolSetting.Set({s,v});
}

BoundSetting::BoundSetting(const IntSetting s, IntSetting::ValueType v){
  m_intSetting.Set({s,v});
}

BoundSetting::BoundSetting(const StringSetting& s,
  const StringSetting::ValueType& v)
{
  m_strSetting.Set({s,v});
}

BoundSetting::BoundSetting(const FloatSetting& s, FloatSetting::ValueType v){
  m_floatSetting.Set({s,v});
}

BoundSetting::BoundSetting(const PaintSetting& s,
  const PaintSetting::ValueType& v)
{
  m_paintSetting.Set({s,v});
}

} // namespace
