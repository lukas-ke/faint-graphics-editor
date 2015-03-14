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

#ifndef FAINT_OBJECT_HANDLE_HH
#define FAINT_OBJECT_HANDLE_HH

namespace faint{

using object_handle_t = int;

enum class Handle{
  P0 = 0,
  P1 = 1,
  P2 = 2,
  P3 = 3,
  P0P2 = 4,
  P1P3 = 5,
  P0P1 = 6,
  P2P3 = 7
};

bool corner_handle(Handle);

} // namespace

#endif
