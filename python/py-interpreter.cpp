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

#include "app/app-context.hh"
#include "python/mapped-type.hh"
#include "python/py-include.hh"
#include "python/py-ugly-forward.hh"
#include "python/py-add-type-object.hh"

namespace faint{

extern PyTypeObject FaintInterpreterType;

struct faintInterpreterObject{
  PyObject_HEAD
  AppContext* ctx;
};

template<>
struct MappedType<AppContext&>{
  using PYTHON_TYPE = faintInterpreterObject;
  static AppContext& GetCppObject(faintInterpreterObject* self){
    return *self->ctx;
  }

  static bool Expired(faintInterpreterObject*){
    // Always ok
    return false;
  }

  static void ShowError(faintInterpreterObject*){
  // Never broken
  }

  static utf8_string DefaultRepr(const faintInterpreterObject*){
    return "FaintInterpreter";
  }
};

void add_interpreter_to_module(PyObject* module){
  FaintInterpreterType.tp_new = PyType_GenericNew;
  int result = PyType_Ready(&FaintInterpreterType);
  assert(result >= 0);
  Py_INCREF(&FaintInterpreterType);
  PyModule_AddObject(module, "FaintInterpreter",
    (PyObject*)&FaintInterpreterType);
}

/* method: "maximize()\n
Maximize or de-maximize the interpreter window." */
static void faintinterpreter_maximize(AppContext& app){
  app.MaximizePythonConsole();
}

/* method: "set_background_color(r,g,b)\n
Set the background color of the interpreter window" */
static void faintinterpreter_set_background_color(AppContext& app,
  const ColRGB& c)
{
  app.SetPythonConsoleBackground(c);
}

/* method: "set_text_color(r,g,b)\n
Sets the text color of the interpreter window." */
static void faintinterpreter_set_text_color(AppContext& app, const ColRGB& c){
  app.SetPythonConsoleTextColor(c);
}

/* method: "__copy__()"
name: "__copy__" */
static void faintinterpreter_copy(AppContext&){
  throw NotImplementedError("FaintInterpreter object can not be copied.");
}

static void faintinterpreter_init(faintInterpreterObject&){
  // Prevent instantiation from Python, since the AppContext can't be
  // provided from there.
  throw TypeError(space_sep("FaintInterpreter can not be instantiated.",
    "Use the 'interpreter'-object instead."));
}

static PyObject* faintinterpreter_new(PyTypeObject* type, PyObject*, PyObject*){
  faintInterpreterObject* self = (faintInterpreterObject*)type->tp_alloc(type, 0);
  return (PyObject*)self;
}

static utf8_string faintinterpreter_repr(AppContext&){
  return "FaintInterpreter";
}

#include "generated/python/method-def/py-interpreter-method-def.hh"

PyTypeObject FaintInterpreterType = {
  PyVarObject_HEAD_INIT(nullptr, 0)
  "FaintInterpreter", // tp_name
  sizeof(faintInterpreterObject), // tp_basicsize
  0, // tp_itemsize
  nullptr, // tp_dealloc
  nullptr, // tp_print
  nullptr, // tp_getattr
  nullptr, // tp_setattr
  nullptr, // tp_compare
  REPR_FORWARDER(faintinterpreter_repr), // tp_repr
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
  "Faint Interpreter Interface", // tp_doc
  nullptr, // tp_traverse
  nullptr, // tp_clear
  nullptr, // tp_richcompare
  0, // tp_weaklistoffset
  nullptr, // tp_iter
  nullptr, // tp_iternext
  faintinterpreter_methods, // tp_methods
  nullptr, // tp_members
  nullptr, // tp_getset
  nullptr, // tp_base
  nullptr, // tp_dict
  nullptr, // tp_descr_get
  nullptr, // tp_descr_set
  0, // tp_dictoffset
  INIT_FORWARDER(faintinterpreter_init), // tp_init
  nullptr, // tp_alloc
  faintinterpreter_new, // tp_new
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

void add_Interpreter(AppContext& app, PyObject* module){
  add_type_object(module, FaintInterpreterType, "FaintInterpreter");
  PyModule_AddObject(module, "interpreter",
    create_python_object<faintInterpreterObject>(FaintInterpreterType, app));
}

} // namespace
