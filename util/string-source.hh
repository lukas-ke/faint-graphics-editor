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

#ifndef FAINT_STRING_SOURCE_HH
#define FAINT_STRING_SOURCE_HH
#include <vector>
#include "text/utf8-string.hh"

namespace faint{

class StringSource{
public:
  virtual ~StringSource(){}
  virtual std::vector<utf8_string> Get() const = 0;
};

} // namespace

#endif
