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

#ifndef FAINT_SELECT_RASTER_RECTANGLE_HH
#define FAINT_SELECT_RASTER_RECTANGLE_HH
#include "util/raster-selection.hh"

namespace faint{

class ActiveCanvas;
class Task;

// Creates a task which draws a raster selection rectangle originating
// at startPos.
Task* raster_selection_rectangle_task(const Point& startPos,
  Settings&,
  bool appendCommand,
  const ActiveCanvas&);

// Variant which includes a selection options update with the
// selection rectangle command, so that the settings (especially
// background color) of the new selection match the application wide
// settings rather than the old (possibly Empty()) selection.
Task* raster_selection_rectangle_task(const Point& startPos,
  Settings&,
  const OldSelectionOptions&,
  bool appendCommand,
  const ActiveCanvas&);

} // namespace

#endif
