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

#ifndef FAINT_SELECT_RASTER_MOVE_HH
#define FAINT_SELECT_RASTER_MOVE_HH
#include "util/raster-selection.hh"

namespace faint{

class Canvas;
class IntPoint;
class Settings;
class Task;

// Task for either selecting a new region by moving the selection
// rectangle, or to move the selected content.
Task* raster_selection_move_task(const IntPoint& offset,
  const IntPoint& topLeft,
  const copy_selected&,
  bool floatSelected,
  Settings&,
  Canvas&);

} // namespace

#endif
