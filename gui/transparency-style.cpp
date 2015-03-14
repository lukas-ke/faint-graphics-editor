// -*- coding: us-ascii-unix -*-
// Copyright 2013 Lukas Kemmer
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

#include "gui/transparency-style.hh"

namespace faint{

TransparencyStyle::TransparencyStyle()
{}

TransparencyStyle::TransparencyStyle(const ColRGB& color)
  : m_color(color)
{}

bool TransparencyStyle::IsCheckered() const{
  return !IsColor();
}

bool TransparencyStyle::IsColor() const{
  return m_color.IsSet();
}

ColRGB TransparencyStyle::GetColor() const{
  return m_color.Get();
}

} // namespace
