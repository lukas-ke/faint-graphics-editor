// -*- coding: us-ascii-unix -*-
// Copyright 2015 Lukas Kemmer
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
#include "text/formatting.hh"
#include "util/setting-id.hh"
#include "python/py-include.hh"
#include "python/py-ugly-forward.hh"
#include "python/py-util.hh"
#include "python/py-func-context.hh"
#include "python/py-add-type-object.hh"
#include "generated/python/settings/setting-functions.hh"

namespace faint{

// ActiveSettings: Python class with functions changing the
// current tool settings in Faint.
//
// This class can only be instantiated from C++, as it needs
// access to the AppContext.
//
// A single object should be added to a Python-module using
// add_ActiveSettings during initialization.
//
// The functions are generated.

extern PyTypeObject ActiveSettingsType;

struct activeSettingsObject{
  PyObject_HEAD
  AppContext* ctx;
};

const char* ACTIVE_SETTINGS_NAME = "ActiveSettings";

template<>
struct MappedType<AppContext&>{
  using PYTHON_TYPE = activeSettingsObject;

  static AppContext& GetCppObject(activeSettingsObject* self){
    return *self->ctx;
  }

  static bool Expired(activeSettingsObject*){
    return false;
  }

  static void ShowError(activeSettingsObject*){}
};

static PyObject* as_new(PyTypeObject* type, PyObject*, PyObject*){
  activeSettingsObject *self;
  self = (activeSettingsObject*)type->tp_alloc(type, 0);
  self->ctx = nullptr;
  return (PyObject *)self;
}

static void as_init(AppContext&){
  // Prevent instantiation from Python, since the expected context
  // can't be provided from there.
  throw TypeError(space_sep(ACTIVE_SETTINGS_NAME, "can not be instantiated."));
}

static PyObject* as_repr(activeSettingsObject*){
  return build_unicode(utf8_string(ACTIVE_SETTINGS_NAME));
}

PyTypeObject ActiveSettingsType = {
  PyVarObject_HEAD_INIT(nullptr, 0)
  ACTIVE_SETTINGS_NAME, // tp_name
  sizeof(activeSettingsObject), // tp_basicsize
  0, // tp_itemsize
  nullptr, // tp_dealloc
  0, // tp_vectorcall_offset
  nullptr, // tp_getattr
  nullptr, // tp_setattr
  nullptr, // tp_as_async
  (reprfunc)as_repr, // tp_repr
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
  "Methods for modifying the active settings.\n", // tp_doc
  nullptr, // tp_traverse
  nullptr, // tp_clear
  nullptr, // tp_richcompare
  0, // tp_weaklistoffset
  nullptr, // tp_iter
  nullptr, // tp_iternext
  active_settings_methods, // tp_methods
  nullptr, // tp_members
  nullptr, // tp_getset
  nullptr, // tp_base
  nullptr, // tp_dict
  nullptr, // tp_descr_get
  nullptr, // tp_descr_set
  0, // tp_dictoffset
  INIT_FORWARDER(as_init), // tp_init
  nullptr, // tp_alloc
  as_new, // tp_new
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

void add_ActiveSettings(AppContext& app, PyObject* module){
  add_type_object(module, ActiveSettingsType, "ActiveSettings");
  PyModule_AddObject(module, "active_settings",
    create_python_object<activeSettingsObject>(ActiveSettingsType, app));
}

} // namespace
