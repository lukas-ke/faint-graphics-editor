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

#ifndef FAINT_TEXT_EXPRESSION_CONVERSIONS_HH
#define FAINT_TEXT_EXPRESSION_CONVERSIONS_HH
#include <map>
#include "geo/primitive.hh"
#include "text/utf8-string.hh"
#include "util/optional.hh"

namespace faint{

using unit_map_t = std::map<utf8_string, coord>;

class Conversions{
public:
  Conversions(const unit_map_t&);
  Optional<coord> operator[](const utf8_string& unit) const;
  Conversions operator/(coord scale) const;
private:
  unit_map_t m_conversions;
};

using conversions_map_t = std::map<utf8_string, Conversions>;

const conversions_map_t& length_conversions();

} // namespace

#endif
