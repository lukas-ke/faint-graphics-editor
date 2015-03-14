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

#ifndef FAINT_POS_INFO_CONSTANTS_HH
#define FAINT_POS_INFO_CONSTANTS_HH

namespace faint{

enum class Hit{
  NONE = 0,
  INSIDE = 1,
  NEARBY,
  BOUNDARY,
};

enum class HandleType{
  MOVABLE_POINT,
  EXTENSION_POINT
};

enum TABLET_CURSORS{
  TABLET_CURSOR_PUCK = 0,
  TABLET_CURSOR_STYLUS = 1,
  TABLET_CURSOR_ERASER = 2
};

enum class MouseButton{
  LEFT,
  RIGHT,
  NONE
};

} // namespace

#endif
