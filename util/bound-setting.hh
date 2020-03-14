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
#include <variant>
#include "util/settings.hh"
#include "util/type-dependent-false.hh"

namespace faint{

class BoundSetting{
private:
  template<typename T>
  using PairType = std::pair<T, typename T::ValueType>;

public:
  BoundSetting(const BoolSetting, BoolSetting::ValueType);
  BoundSetting(const IntSetting, IntSetting::ValueType);
  BoundSetting(const StringSetting&, const StringSetting::ValueType&);
  BoundSetting(const FloatSetting&, FloatSetting::ValueType);
  BoundSetting(const PaintSetting&, const PaintSetting::ValueType&);

  template<typename T>
  BoundSetting(const EnumSetting<T>& s, const typename EnumSetting<T>::ValueType& v)
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
    return std::visit(
      [&](auto&& arg){

        const auto& [setting, value] = arg;
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, PairType<BoolSetting>>) {
          return boolFunc(setting, value);
        }
        else if constexpr (std::is_same_v<T, PairType<IntSetting>>) {
          return intFunc(setting, value);
        }
        else if constexpr (std::is_same_v<T, PairType<StringSetting>>) {
          return strFunc(setting, value);
        }
        else if constexpr (std::is_same_v<T, PairType<FloatSetting>>) {
          return floatFunc(setting, value);
        }
        else if constexpr (std::is_same_v<T, PairType<PaintSetting>>) {
          return colorFunc(setting, value);
        }
        else {
          static_assert(TypeDependentFalse_v<T>, "Non-exhaustive variant visitor");
        }
      }, m_setting);
  }

  template<typename T>
  auto Visit(T f) const{
    return Visit(f, f, f, f, f);
  }


private:
  std::variant<
    PairType<BoolSetting>,
    PairType<IntSetting>,
    PairType<StringSetting>,
    PairType<FloatSetting>,
    PairType<PaintSetting>> m_setting;
};

} // namespace

#endif
