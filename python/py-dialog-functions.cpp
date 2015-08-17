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

#include "app/faint-resize-dialog-context.hh"
#include "app/get-app-context.hh" // Fixme: Remove
#include "gui/dialogs.hh"
#include "python/py-dialog-functions.hh"
#include "text/formatting.hh"
#include "python/py-ugly-forward.hh"
#include "python/py-util.hh"

namespace faint{

const char* DIALOG_FUNCTIONS_NAME = "DialogFunctions";

template<>
struct MappedType<PyFuncContext&>{
  using PYTHON_TYPE = dialogFunctionsObject;

  static PyFuncContext& GetCppObject(dialogFunctionsObject* self){
    return *self->ctx;
  }

  static bool Expired(dialogFunctionsObject*){
    return false;
  }

  static void ShowError(dialogFunctionsObject*){}
};

// Fixme: Remove all get_app_context, pass via PyFuncContext.

/* method: "brightness_contrast()\n
Show the brightness/contrast dialog." */
static void dialog_brightness_contrast(PyFuncContext&){
  get_app_context().Modal(show_brightness_contrast_dialog);
}

/* method: "color_balance()\n
Show the color balance dialog." */
static void dialog_color_balance(PyFuncContext&){
  AppContext& app = get_app_context();
  app.Modal(show_color_balance_dialog);
}

/* method: "help()\n
Show the help window." */
static void dialog_help(PyFuncContext&){
  get_app_context().ToggleHelpFrame();
}

/* method: "open_file()\n
Show the open file dialog." */
static void dialog_open_file(PyFuncContext&){
  get_app_context().DialogOpenFile();
}

/* method: "resize()\n
Show the image/selection resize dialog." */
static void dialog_resize(PyFuncContext& ctx){
  show_resize_dialog(get_app_context(), ctx.art);
}

/* method: "rotate()\n
Show the rotation dialog (for rotating the image or selection)." */
static void dialog_rotate(PyFuncContext& ctx){
  AppContext& app = get_app_context();
  app.ModalFull(bind_show_rotate_dialog(ctx.art, app.GetDialogContext()));
}

/* method: "save_copy()\n
Show the save file dialog, without modifying the current file name
when saved." */
static void dialog_save_copy(PyFuncContext&, Canvas* c){
  get_app_context().DialogSaveAs(*c, true);
}

/* method: "save_file()\n
Show the save file dialog." */
static void dialog_save_file(PyFuncContext&, Canvas* c){
  get_app_context().DialogSaveAs(*c, false);
}

/* method: "threshold()\n
Show the threshold dialog." */
static void dialog_threshold(PyFuncContext&){
  AppContext& app = get_app_context();
  // const Settings& s = app.GetToolSettings();
  // Fixme: Pass settings?
  show_threshold_dialog(app.GetDialogContext(), app.GetToolSettings());
}

/* method: "python_console()\n
Show the Python console." */
static void dialog_python_console(PyFuncContext&){
  get_app_context().TogglePythonConsole();
}

static PyObject* dialog_functions_new(PyTypeObject* type, PyObject*, PyObject*){
  dialogFunctionsObject *self;
  self = (dialogFunctionsObject*)type->tp_alloc(type, 0);
  self->ctx = nullptr;
  return (PyObject *)self;
}

static void dialog_init(PyFuncContext&){
  // Prevent instantiation from Python,, since the expected context
  // can't be provided from there.
  throw TypeError(space_sep(DIALOG_FUNCTIONS_NAME, "can not be instantiated."));
}

static PyObject* dialog_repr(dialogFunctionsObject*){
  return build_unicode(utf8_string(DIALOG_FUNCTIONS_NAME));
}

#include "generated/python/method-def/py-dialog-functions-methoddef.hh"

PyTypeObject DialogFunctionsType = {
  PyVarObject_HEAD_INIT(nullptr, 0)
  DIALOG_FUNCTIONS_NAME, // tp_name
  sizeof(dialogFunctionsObject), // tp_basicsize
  0, // tp_itemsize
  nullptr, // tp_dealloc
  nullptr, // tp_print
  nullptr, // tp_getattr
  nullptr, // tp_setattr
  nullptr, // reserved (formerly tp_compare)
  (reprfunc)dialog_repr, // tp_repr
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
  "Methods for showing Faint dialogs.\n", // tp_doc
  nullptr, // tp_traverse
  nullptr, // tp_clear
  nullptr, // tp_richcompare
  0, // tp_weaklistoffset
  nullptr, // tp_iter
  nullptr, // tp_iternext
  dialog_functions_methods, // tp_methods
  nullptr, // tp_members
  nullptr, // tp_getset
  nullptr, // tp_base
  nullptr, // tp_dict
  nullptr, // tp_descr_get
  nullptr, // tp_descr_set
  0, // tp_dictoffset
  INIT_FORWARDER(dialog_init), // tp_init
  nullptr, // tp_alloc
  dialog_functions_new, // tp_new
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

PyObject* create_dialog_functions(PyFuncContext& ctx){
  auto* py_obj = (dialogFunctionsObject*)
    DialogFunctionsType.tp_alloc(&DialogFunctionsType, 0);
  py_obj->ctx = &ctx;
  return (PyObject*)py_obj;
}

} // namespace
