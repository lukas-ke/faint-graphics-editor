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
class PyFuncContext;

class CanvasGrid{
public:
  explicit CanvasGrid(PyFuncContext& ctx, CanvasId canvasId)
    : ctx(ctx),
      canvasId(canvasId),
      targetActive(false)
  {}

  explicit CanvasGrid(PyFuncContext& ctx)
    : ctx(ctx),
      targetActive(true)
  {}

  Canvas& GetCanvas() const;

  Grid GetGrid() const;
  void SetGrid(const Grid&) const;

  PyFuncContext& ctx;
  CanvasId canvasId;
  bool targetActive;
};

extern PyTypeObject GridType;

struct gridObject {
  // Fixme: Move into impl..
  PyObject_HEAD
  PyFuncContext* ctx;
  bool targetActive;
  CanvasId canvasId;
};

// Returns a grid object targetting the specified canvas.
PyObject* py_grid_canvas(PyFuncContext&, CanvasId id);

// Returns a grid always targetting the active canvas.
PyObject* py_active_grid(PyFuncContext&);

PyObject* py_grid(const CanvasGrid&);

// Returns the Grid object from this gridObject
// Sets a Python-error unless the grid is OK.
Optional<Grid> get_grid(gridObject*);

} // namespace

#endif
