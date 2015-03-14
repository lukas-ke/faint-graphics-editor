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

#ifndef FAINT_SIZE_HH
#define FAINT_SIZE_HH
#include "geo/primitive.hh"

namespace faint{

class Size{
public:
  constexpr Size(coord w, coord h) : w(w), h(h){}
  static constexpr Size Both(coord v){ return Size(v,v); }
  bool operator==(const Size&) const;
  bool operator!=(const Size&) const;
  Size operator/(const Size&) const;
  using value_type = coord;
  value_type w;
  value_type h;
};

coord area(const Size&);
bool area_less(const Size& lhs, coord rhs);
Size operator+(const Size&, const Size&);
Size operator-(const Size&, const Size&);
Size operator/(const Size&, coord);

} // namespace

#endif
