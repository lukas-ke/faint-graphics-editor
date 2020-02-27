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

#include <sstream>
#include "app/app-context.hh"
#include "app/canvas.hh"
#include "python/mapped-type.hh"
#include "python/py-add-type-object.hh"
#include "python/py-include.hh"
#include "python/py-grid.hh"
#include "python/py-ugly-forward.hh"
#include "util/grid.hh"
#include "util/optional.hh"

namespace faint{

Canvas& CanvasGrid::GetCanvas() const{
  return targetActive ?
    ctx.app.GetActiveCanvas() : ctx.app.GetCanvas(canvasId);
}

Grid CanvasGrid::GetGrid() const{
  return GetCanvas().GetGrid();
}

void CanvasGrid::SetGrid(const Grid& g) const{
  Canvas& c = GetCanvas();
  c.SetGrid(g);
  ctx.py.QueueRefresh(c);
}

extern PyTypeObject GridType;

struct gridObject {
  PyObject_HEAD
  PyFuncContext* ctx;
  bool targetActive;
  CanvasId canvasId;
};

static bool grid_ok_no_error(gridObject* self){
  if (self->targetActive){
    // The grid for the active canvas is the target, there should always be
    // at least one canvas.
    return true;
  }
  else{
    return self->ctx->app.Exists(self->canvasId);
  }
}

template<>
struct MappedType<const CanvasGrid&>{
  using PYTHON_TYPE = gridObject;

  static CanvasGrid GetCppObject(gridObject* self){
    if (self->targetActive){
      return CanvasGrid(*self->ctx);
    }
    return CanvasGrid(*self->ctx, self->canvasId);
  }

  static bool Expired(gridObject* grid){
    return !grid_ok_no_error(grid);
  }

  static void ShowError(gridObject*){
    PyErr_SetString(PyExc_ValueError, "Canvas for grid removed.");
  }
};

// Returns true if the canvas for the grid exists.
// Otherwise, sets a python error and returns false.
bool grid_ok(gridObject* self){
  if (grid_ok_no_error(self)){
    return true;
  }
  PyErr_SetString(PyExc_ValueError, "Canvas for grid removed.");
  return false;
}

static Canvas& get_canvas(gridObject* self){
  return self->targetActive ?
    self->ctx->app.GetActiveCanvas() :
    self->ctx->app.GetCanvas(self->canvasId);
}

Optional<Grid> get_grid(PyObject* obj){
  if (!PyObject_IsInstance(obj, (PyObject*)&GridType)){
    return {};
  }

  auto grid = (gridObject*)obj;
  if (grid_ok(grid)){
    return option(get_canvas(grid).GetGrid());
  }
  else{
    return {};
  }
}

static PyObject* grid_new(PyTypeObject* type, PyObject*, PyObject*){
  gridObject* self = (gridObject*)type->tp_alloc(type, 0);
  return (PyObject*)self;
}

static void grid_init(gridObject&){
  // TODO: Create active_grid in py-init.
  throw TypeError("Grid can not be instantiated. "
    "Use canvas.grid or app.grid instead.");
}

static PyObject* grid_repr(gridObject* self){
  if (self->targetActive){
    return Py_BuildValue("s", "Grid (active canvas)");
  }

  std::stringstream ss;
  if (grid_ok_no_error(self)){
    ss << "Grid for Canvas #" << self->canvasId.Raw();
  }
  else{
    ss << "Grid for removed Canvas #" << self->canvasId.Raw();
  }
  return Py_BuildValue("s", ss.str().c_str());
}

/* property: "grid_anchor (x,y-tuple)\n
Specifies a point that will be intersected by the grid." */
struct grid_anchor{
  static Point Get(const CanvasGrid& self){
    return self.GetGrid().Anchor();
  }

  static void Set(const CanvasGrid& self, const Point& anchor){
    Grid g = self.GetGrid();
    g.SetAnchor(anchor);
    self.SetGrid(g);
  }
};

/* property: "color (rgba-tuple)\n
The color of the Grid lines." */
struct grid_color{
  static Color Get(const CanvasGrid& self){
    return self.GetGrid().GetColor();
  }

  static void Set(const CanvasGrid& self, const Color& c){
    Grid g = self.GetGrid();
    g.SetColor(c);
    self.SetGrid(g);
  }
};

/* property: "dashed (boolean)\n
Whether the grid lines are dashed or solid." */
struct grid_dashed{
  static bool Get(const CanvasGrid& self){
    return self.GetGrid().Dashed();
  }

  static void Set(const CanvasGrid& self, bool dashed){
    Grid g = self.GetGrid();
    g.SetDashed(dashed);
    self.SetGrid(g);
  }
};

/* property: "spacing (int)\n
The spacing between the grid lines in pixels." */
struct grid_spacing{
  static int Get(const CanvasGrid& self){
    return self.GetGrid().Spacing();
  }

  static void Set(const CanvasGrid& self, int spacing){
    Grid g = self.GetGrid();
    g.SetSpacing(spacing);
    self.SetGrid(g);
  }
};

/* property: "enabled (boolean)\n
Specifies if this grid is visible and enabled." */
struct grid_enabled{
  static bool Get(const CanvasGrid& self){
    return self.GetGrid().Enabled();
  }

  static void Set(const CanvasGrid& self, bool enabled){
    Grid g = self.GetGrid();
    g.SetEnabled(enabled);
    self.SetGrid(g);

  }
};

/* method: "__copy__() Not implemented."
name: "__copy__" */
static void grid_copy(const CanvasGrid&){
  throw NotImplementedError("Grid can not be copied.");
}

#include "generated/python/method-def/py-grid-method-def.hh"

PyTypeObject GridType = {
  PyVarObject_HEAD_INIT(nullptr, 0)
  "Grid", // tp_name
  sizeof(gridObject), //tp_basicsize
  0, // tp_itemsize
  nullptr, // tp_dealloc
  0, // tp_vectorcall_offset
  nullptr, // tp_getattr
  nullptr, // tp_setattr
  nullptr, // tp_compare
  (reprfunc)grid_repr, // tp_repr
  nullptr, // tp_as_number
  nullptr, // tp_as_sequence
  nullptr, // tp_as_mapping
  nullptr, // tp_hash
  nullptr, // tp_call
  nullptr, // tp_str
  nullptr, // tp_getattro
  nullptr, // tp_setattro
  nullptr, // tp_as_buffer
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags
  "Interface for adjusting the guiding/snapping grids in an image.", // tp_doc
  nullptr, // tp_traverse */
  nullptr, // tp_clear */
  nullptr, // tp_richcompare */
  0, // tp_weaklistoffset */
  nullptr, // tp_iter */
  nullptr, // tp_iternext */
  grid_methods, // tp_methods */
  nullptr, // tp_members
  grid_getseters, // tp_getset */
  nullptr, // tp_base */
  nullptr, // tp_dict */
  nullptr, // tp_descr_get */
  nullptr, // tp_descr_set */
  0, // tp_dictoffset */
  INIT_FORWARDER(grid_init), // tp_init
  nullptr, // tp_alloc
  grid_new, // tp_new */
  nullptr, // tp_free
  nullptr, // tp_is_gc
  nullptr, // tp_bases
  nullptr, // tp_mro
  nullptr, // tp_cache
  nullptr, // tp_subclasses
  nullptr, // tp_weaklist
  nullptr, // tp_del
  0, // tp_version_tag
  nullptr  // tp_finalize
};

// Returns a grid always targetting the active canvas.
PyObject* py_grid(const CanvasGrid& grid){
  gridObject* py_grid = (gridObject*) GridType.tp_alloc(&GridType,0);
  py_grid->ctx = &grid.ctx;
  py_grid->targetActive = grid.targetActive;
  py_grid->canvasId = grid.canvasId;
  return (PyObject*) py_grid;
}

// Returns a grid object targetting the specified canvas.
PyObject* py_grid_canvas(PyFuncContext& ctx, CanvasId id){
  gridObject* py_grid = (gridObject*) GridType.tp_alloc(&GridType,0);
  py_grid->ctx = &ctx;
  py_grid->targetActive = false;
  py_grid->canvasId = id;
  return (PyObject*) py_grid;
}

// Returns a grid always targetting the active canvas.
PyObject* py_active_grid(PyFuncContext& ctx){
  gridObject* py_grid = (gridObject*) GridType.tp_alloc(&GridType,0);
  py_grid->ctx = &ctx;
  py_grid->targetActive = true;
  return (PyObject*) py_grid;
}

void add_type_Grid(PyObject* module){
  add_type_object(module, GridType, "Grid");
}

bool is_Grid(PyObject* obj){
  return PyObject_IsInstance(obj, (PyObject*)&GridType) != 0;
}

} // namespace
