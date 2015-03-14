// -*- coding: us-ascii-unix -*-
// Copyright 2015 Lukas Kemmer
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

#include <algorithm>
#include "geo/int-range.hh"

namespace faint{

IntRange::IntRange(const max_t& max)
  :  m_max(max.Get())
{}

IntRange::IntRange(const min_t& min)
  : m_min(min.Get())
{}

IntRange::IntRange(const min_t& min, const max_t& max)
  : m_max(max.Get()),
    m_min(min.Get())
{
  assert(m_min.Get() <= m_max.Get());
}

int IntRange::Constrain(int value) const{
  if (m_min.IsSet()){
    value = std::max(m_min.Get(), value);
  }
  if (m_max.IsSet()){
    value = std::min(m_max.Get(), value);
  }
  return value;
}

bool IntRange::Has(int value) const{
  if (m_min.IsSet() && value < m_min.Get()){
    return false;
  }
  if (m_max.IsSet() && m_max.Get() < value){
    return false;
  }
  return true;
}

}
