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
  BoundSetting(const PaintSetting&, const PaintSetting::ValueType&);
  template<typename T>
  BoundSetting(const EnumSetting<T>& s,
    const typename EnumSetting<T>::ValueType& v)
    : BoundSetting(static_cast<IntSetting>(s), to_int(v))
  {}

  template<typename BoolFunc, typename IntFunc, typename StrFunc,
           typename FloatFunc, typename ColorFunc>
  auto Visit(const BoolFunc& boolFunc,
    const IntFunc& intFunc,
    const StrFunc& strFunc,
    const FloatFunc& floatFunc,
    const ColorFunc& colorFunc) const
  {
    if (m_boolSetting.IsSet()){
      const auto& [setting, value] = m_boolSetting.Get();
      return boolFunc(setting, value);
    }
    else if (m_intSetting.IsSet()){
      const auto& [setting, value] = m_intSetting.Get();
      return intFunc(setting, value);
    }
    else if (m_strSetting.IsSet()){
      const auto& [setting, value] = m_strSetting.Get();
      return strFunc(setting, value);
    }
    else if (m_floatSetting.IsSet()){
      const auto& [setting, value] = m_floatSetting.Get();
      return floatFunc(setting, value);
    }
    else{
      assert(m_paintSetting.IsSet());
      const auto& [setting, value] = m_paintSetting.Get();
      return colorFunc(setting, value);
    }
  }

  template<typename T>
  auto Visit(T f) const{
    return Visit(f, f, f, f, f);
  }


private:
  Optional<std::pair<BoolSetting, BoolSetting::ValueType> > m_boolSetting;
  Optional<std::pair<IntSetting, IntSetting::ValueType> > m_intSetting;
  Optional<std::pair<StringSetting, StringSetting::ValueType> > m_strSetting;
  Optional<std::pair<FloatSetting, FloatSetting::ValueType> > m_floatSetting;
  Optional<std::pair<PaintSetting, PaintSetting::ValueType> > m_paintSetting;
};

} // namespace

#endif
