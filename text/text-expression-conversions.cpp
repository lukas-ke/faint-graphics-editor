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

#include "text/text-expression-conversions.hh"

namespace faint{

Conversions::Conversions(const unit_map_t& c)
  : m_conversions(c)
{}

Optional<coord> Conversions::operator[](const utf8_string& unit) const{
  auto it = m_conversions.find(unit);
  if (it == end(m_conversions)){
    return {};
  }
  return option(it->second);
}

Conversions Conversions::operator/(coord scale) const{
  unit_map_t other(m_conversions);
  for (auto& c : other){
    c.second /= scale;
  }
  return Conversions(other);
}

static auto rescale(const utf8_string& unit, const Conversions& conv){
  return std::make_pair(unit, conv / conv[unit].Get());
}

const conversions_map_t& length_conversions(){
  static const Conversions mmPer{{
    {"mm", 1.0},
    {"cm", 10.0},
    {"dm", 100.0},
    {"m", 1000.0},
    {"km", 1000000.0},
    {"in", 25.4}}};

  static const conversions_map_t conversions{{
    {"mm", mmPer},
    rescale("cm", mmPer),
    rescale("dm", mmPer),
    rescale("m", mmPer),
    rescale("km", mmPer),
    rescale("in", mmPer)
  }};
  return conversions;
}

} // namespace
