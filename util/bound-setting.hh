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

#ifndef FAINT_BOUND_SETTING_HH
#define FAINT_BOUND_SETTING_HH
#include "util/optional.hh"
#include "util/settings.hh"

namespace faint{

class BoundSetting{
public:
  BoundSetting(const BoolSetting, BoolSetting::ValueType);
  BoundSetting(const IntSetting, IntSetting::ValueType);
  BoundSetting(const StringSetting&, const StringSetting::ValueType&);
  BoundSetting(const FloatSetting&, FloatSetting::ValueType);
  BoundSetting(const ColorSetting&, const ColorSetting::ValueType&);
  template<typename T>
  BoundSetting(const EnumSetting<T>& s,
    const typename EnumSetting<T>::ValueType& v)
    : BoundSetting(static_cast<IntSetting>(s), to_int(v))
  {}
  template<typename BoolFunc, typename IntFunc, typename StrFunc,
           typename FloatFunc, typename ColorFunc>
  bool Visit(const BoolFunc& boolFunc,
    const IntFunc& intFunc,
    const StrFunc& strFunc,
    const FloatFunc& floatFunc,
    const ColorFunc& colorFunc) const
  {

    if (m_boolSetting.IsSet()){
      const auto& p = m_boolSetting.Get();
      return boolFunc(p.first, p.second);
    }
    else if (m_intSetting.IsSet()){
      const auto& p = m_intSetting.Get();
      return intFunc(p.first, p.second);
    }
    else if (m_strSetting.IsSet()){
      const auto& p = m_strSetting.Get();
      return strFunc(p.first, p.second);
    }
    else if (m_floatSetting.IsSet()){
      const auto& p = m_floatSetting.Get();
      return floatFunc(p.first, p.second);
    }
    else{
      assert(m_colorSetting.IsSet());
      const auto& p = m_colorSetting.Get();
      return colorFunc(p.first, p.second);
    }
  }
private:
  Optional<std::pair<BoolSetting, BoolSetting::ValueType> > m_boolSetting;
  Optional<std::pair<IntSetting, IntSetting::ValueType> > m_intSetting;
  Optional<std::pair<StringSetting, StringSetting::ValueType> > m_strSetting;
  Optional<std::pair<FloatSetting, FloatSetting::ValueType> > m_floatSetting;
  Optional<std::pair<ColorSetting, ColorSetting::ValueType> > m_colorSetting;
};

} // namespace

#endif
