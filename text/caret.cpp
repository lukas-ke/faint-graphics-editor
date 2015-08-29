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

#include "text/caret.hh"

namespace faint{

CaretRange::CaretRange(Caret from, Caret to)
  : from(from),
    to(to)
{}

CaretRange CaretRange::Both(Caret v){
  return CaretRange(v,v);
}

bool CaretRange::Empty() const{
  return from == to;
}

bool CaretRange::operator==(const CaretRange& other) const{
  return from == other.from && to == other.to;
}

bool CaretRange::operator!=(const CaretRange& other) const{
  return !operator==(other);
}

} // namespace
