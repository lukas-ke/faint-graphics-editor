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

#include "util/convenience.hh"

namespace faint{

bool fl(int flag, int bits){
  return (bits & flag) == flag;
}

bool neither(bool a, bool b){
  return !(a || b);
}

bool then_false(bool& value){
  if (value){
    value = false;
    return true;
  }
  return false;
}

} // namespace
