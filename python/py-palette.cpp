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

#include <vector>
#include "app/app-context.hh"
#include "bitmap/paint.hh"
#include "util/paint-map.hh"
#include "python/mapped-type.hh"
#include "python/py-include.hh"
#include "python/py-ugly-forward.hh"
#include "python/py-add-type-object.hh"

namespace faint{

extern PyTypeObject FaintPaletteType;

struct faintPaletteObject{
  PyObject_HEAD
  AppContext* ctx;
};

template<>
struct MappedType<AppContext&>{
  using PYTHON_TYPE = faintPaletteObject;

  static AppContext& GetCppObject(faintPaletteObject* obj){
    return *obj->ctx;
  }

  static bool Expired(faintPaletteObject*){
    // Always ok
    return false;
  }

  static void ShowError(faintPaletteObject*){
    // Never broken
  }

  static utf8_string DefaultRepr(const faintPaletteObject*){
    return "Faint Palette";
  }
};

/* method: "add(paint)\n
Adds the specified color, gradient or pattern to the palette." */
static void faintpalette_add(AppContext& app, const Paint& paint){
  app.AddToPalette(paint);
}

/* method: "set(paint1,paint2,...)\n
Sets the palette to the specified list of colors, gradients and patterns." */
static void faintpalette_set(AppContext& app, const std::vector<Paint>& paints){
  if (paints.empty()){
    throw ValueError("At least one color required.");
  }
  PaintMap paintMap;
  for (const auto& paint : paints){
    paintMap.Append(paint);
  }
  app.SetPalette(paintMap);
}

static void faintpalette_init(faintPaletteObject&){
  // Prevent instantiation from Python, since the AppContext can't be
  // provided from there.
  throw TypeError(space_sep("Palette can not be instantiated.",
    "Use the 'palette'-object instead."));
}

static PyObject* faintpalette_new(PyTypeObject* type, PyObject*, PyObject*){
  faintPaletteObject* self = (faintPaletteObject*)type->tp_alloc(type, 0);
  return (PyObject *)self;
}

static utf8_string faintpalette_repr(AppContext&){
  return "Faint Palette";
}

/* method: "__copy__()"
name: "__copy__" */
static void faintpalette_copy(AppContext&){
  throw NotImplementedError("Palette object can not be copied.");
}

#include "generated/python/method-def/py-palette-method-def.hh"

PyTypeObject FaintPaletteType = {
    PyVarObject_HEAD_INIT(nullptr, 0)
    "FaintPalette", // tp_name
    sizeof(faintPaletteObject), // tp_basicsize
    0, // tp_itemsize
    nullptr, // tp_dealloc
    nullptr, // tp_print
    nullptr, // tp_getattr
    nullptr, // tp_setattr
    nullptr, // tp_compare
    REPR_FORWARDER(faintpalette_repr), // tp_repr
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
    "Faint Palette interface", // tp_doc
    nullptr, // tp_traverse
    nullptr, // tp_clear
    nullptr, // tp_richcompare
    0, // tp_weaklistoffset
    nullptr, // tp_iter
    nullptr, // tp_iternext
    faintpalette_methods, // tp_methods
    nullptr, // tp_members
    nullptr, // tp_getset
    nullptr, // tp_base
    nullptr, // tp_dict
    nullptr, // tp_descr_get
    nullptr, // tp_descr_set
    0, // tp_dictoffset
    INIT_FORWARDER(faintpalette_init), // tp_init
    nullptr, // tp_alloc
    faintpalette_new, // tp_new
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

void add_Palette(AppContext& app, PyObject* module){
  add_type_object(module, FaintPaletteType, "FaintPalette");
  PyModule_AddObject(module, "palette",
    create_python_object<faintPaletteObject>(FaintPaletteType, app));
}

} // namespace
