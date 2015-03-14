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

#include "app/get-app-context.hh"
#include "python/mapped-type.hh"
#include "python/py-include.hh"
#include "python/py-window.hh"
#include "python/py-ugly-forward.hh"

namespace faint{

template<>
struct MappedType<AppContext&>{
  using PYTHON_TYPE = faintWindowObject;

  static AppContext& GetCppObject(faintWindowObject*){
    return get_app_context();
  }

  static bool Expired(faintWindowObject*){
    // Always ok
    return false;
  }

  static void ShowError(faintWindowObject*){
    // Never broken
  }

  static utf8_string DefaultRepr(const faintWindowObject*){
    return "FaintWindow";
  }
};

void add_window_to_module(PyObject* module){
  FaintWindowType.tp_new = PyType_GenericNew;
  int result = PyType_Ready(&FaintWindowType);
  assert(result >= 0);
  Py_INCREF(&FaintWindowType);
  PyModule_AddObject(module, "FaintWindow", (PyObject*)&FaintWindowType);
}

/* method: "maximize()\n
Maximize or de-maximize the window." */
static void faintwindow_maximize(AppContext& app){
  app.Maximize();
}

static PyObject* faintwindow_new(PyTypeObject* type, PyObject*, PyObject*){
  faintWindowObject* self = (faintWindowObject*)type->tp_alloc(type, 0);
  return (PyObject *)self;
}

static void faintwindow_init(faintWindowObject&){
}

static utf8_string faintwindow_repr(AppContext&){
  return "FaintWindow";
}

#include "generated/python/method-def/py-window-methoddef.hh"

PyTypeObject FaintWindowType = {
    PyVarObject_HEAD_INIT(nullptr, 0)
    "FaintWindow", //tp_name
    sizeof(faintWindowObject), // tp_basicsize
    0, // tp_itemsize
    nullptr, // tp_dealloc
    nullptr, // tp_print
    nullptr, // tp_getattr
    nullptr, // tp_setattr
    nullptr, // tp_compare
    REPR_FORWARDER(faintwindow_repr), // tp_repr
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
    "The Faint Application Window", // tp_doc
    nullptr, // tp_traverse
    nullptr, // tp_clear
    nullptr, // tp_richcompare
    0, // tp_weaklistoffset
    nullptr, // tp_iter
    nullptr, // tp_iternext
    faintwindow_methods, // tp_methods
    nullptr, // tp_members
    nullptr, // tp_getset
    nullptr, // tp_base
    nullptr, // tp_dict
    nullptr, // tp_descr_get
    nullptr, // tp_descr_set
    0, // tp_dictoffset
    INIT_FORWARDER(faintwindow_init), // tp_init
    nullptr, // tp_alloc
    faintwindow_new, // tp_new
    nullptr, // tp_free
    nullptr, // tp_is_gc
    nullptr, // tp_bases
    nullptr, // tp_mro
    nullptr, // tp_cache
    nullptr, // tp_subclasses
    nullptr, // tp_weaklist
    nullptr, // tp_del
    0, // tp_version_tag
    nullptr, // tp_finalize
};

} // namespace
