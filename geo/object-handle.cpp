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

#include "geo/object-handle.hh"

namespace faint{

bool corner_handle(Handle handle){
  // The first four handles are at the corners. The following are edge
  // handles, i.e. for resizing up/down, left/right
  return handle == Handle::P0 || handle == Handle::P1 ||
    handle == Handle::P2 || handle == Handle::P3;
}

}
