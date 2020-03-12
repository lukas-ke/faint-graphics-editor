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

#include "util/bound-setting.hh"

namespace faint{

BoundSetting::BoundSetting(const BoolSetting s, BoolSetting::ValueType v)
  : m_setting(std::pair(s, v))
{}

BoundSetting::BoundSetting(const IntSetting s, IntSetting::ValueType v)
  : m_setting(std::pair(s, v))
{}

BoundSetting::BoundSetting(const StringSetting& s, const StringSetting::ValueType& v)
  : m_setting(std::pair(s, v))
{}

BoundSetting::BoundSetting(const FloatSetting& s, FloatSetting::ValueType v)
  : m_setting(std::pair(s, v))
{}

BoundSetting::BoundSetting(const PaintSetting& s, const PaintSetting::ValueType& v)
  : m_setting(std::pair(s, v))
{}

} // namespace
