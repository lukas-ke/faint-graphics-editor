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

#ifndef FAINT_PY_GRID_HH
#define FAINT_PY_GRID_HH
#include "util/id-types.hh"
#include "util/template-fwd.hh"

namespace faint{

class Canvas;
class Grid;

class CanvasGrid{
public:
  explicit CanvasGrid(Canvas* canvas)
    : canvas(canvas)
  {}
  Canvas* canvas;
};

extern PyTypeObject GridType;

struct gridObject {
  PyObject_HEAD
  Canvas* canvas;
  CanvasId canvasId;
};

// Returns a grid object targetting the specified canvas.
// If the specified canvas is 0, the grid object will
// always target the active canvas.
PyObject* py_grid_canvas(Canvas*);

// Returns the Grid object from this gridObject
// Sets a Python-error unless the grid is OK.
Optional<Grid> get_grid(gridObject*);

} // namespace

#endif
