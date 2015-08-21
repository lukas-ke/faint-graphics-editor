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

#include "app/canvas.hh"
#include "app/app-context.hh"
#include "app/app-getter-util.hh"
#include "app/faint-resize-dialog-context.hh"
#include "geo/int-point.hh"
#include "gui/dialogs.hh"
#include "python/py-global-functions.hh"
#include "text/formatting.hh"
#include "util/setting-id.hh"
#include "objects/objraster.hh"
#include "objects/objtext.hh"
#include "python/python-context.hh"
#include "python/py-ugly-forward.hh"
#include "python/py-util.hh"
#include "python/py-add-type-object.hh"
#include "util/index-iter.hh" // up_to
#include "util/make-vector.hh"
#include "util/key-press.hh"

namespace faint{

extern PyTypeObject GlobalFunctionsType;

struct globalFunctionsObject{
  PyObject_HEAD
  PyFuncContext* ctx;
};

const char* GLOBAL_FUNCTIONS_NAME = "GlobalFunctions";

template<>
struct MappedType<PyFuncContext&>{
  using PYTHON_TYPE = globalFunctionsObject;

  static PyFuncContext& GetCppObject(globalFunctionsObject* self){
    return *self->ctx;
  }

  static bool Expired(globalFunctionsObject*){
    return false;
  }

  static void ShowError(globalFunctionsObject*){}
};

/* method: "Faint internal function."
name: "int_incomplete_command" */
static void f_incomplete_command(PyFuncContext& ctx){
  ctx.py.Continuation();
}

/* method: "Faint internal function." */
static void f_int_bind_key(PyFuncContext& ctx, const KeyPress& keyPress){
  // Inform the C++ side that the key is bound
  ctx.py.Bind(keyPress, bind_global(false));
}

/* method: "Faint internal function." */
static void f_int_bind_key_global(PyFuncContext& ctx,
  const KeyPress& keyPress)
{
  // Inform the C++ side that the key is bound globally
  ctx.py.Bind(keyPress, bind_global(true));
}

/* method: "Faint internal function." */
static void f_int_faint_print(PyFuncContext& ctx, const utf8_string& s){
  ctx.py.IntFaintPrint(s);
}

/* method: "Faint internal function." */
static void f_int_get_key(PyFuncContext& ctx){
  ctx.py.GetKey();
}

/* method: "Faint internal function." */
static utf8_string f_int_get_key_name(PyFuncContext&, int keyCode){
  return KeyPress(Key(keyCode)).Name();
}

/* method: "Faint internal function" */
static void f_int_unbind_key(PyFuncContext& ctx, const KeyPress& keyPress){
  // Inform the C++ side that the key is unbound
  ctx.py.Unbind(keyPress);
}

/* method: "Faint internal function." */
static void f_int_ran_command(PyFuncContext& ctx){
  ctx.py.EvalDone();
  ctx.py.NewPrompt();
}

/* method: "autosize_raster(raster_object)\n
Trims away same-colored borders around a raster object" */
static void f_autosize_raster(PyFuncContext& ctx, BoundObject<ObjRaster>& bound){
  ctx.py.RunCommand(bound.Plain(), crop_raster_object_command(bound.obj));
}

/* method: "autosize_text(text_object)\n
Resizes the object's text box to snugly fit the text." */
static void f_autosize_text(PyFuncContext& ctx, BoundObject<ObjText>& bound){
  ctx.py.RunCommand(bound.Plain(), crop_text_region_command(bound.obj));
}

/* method: "get_active_image()\n
Returns the active (currently edited) image." */
static Canvas& f_get_active_image(PyFuncContext& ctx){
  return ctx.app.GetActiveCanvas();
}

/* method: "get_font()->font\nReturns the active font face name." */
static StringSetting::ValueType f_get_font(PyFuncContext& ctx){
  return ctx.app.Get(ts_FontFace);
}

/* method: "get_layer()->i\nReturns the layer index." */
static int f_get_layer(PyFuncContext& ctx){
  return static_cast<int>(ctx.app.GetLayerType());
}

/* method: "get_mouse_pos_screen()->x,y\nReturns the mouse pointer
position in screen coordinates." */
static IntPoint f_get_mouse_pos_screen(PyFuncContext& ctx){
  return ctx.app.GetMousePos();
}

/* method: "get_settings()\n
Returns a copy of the current tool settings." */
static Settings f_get_settings(PyFuncContext& ctx){
  return ctx.app.GetToolSettings();
}

/* method: "list_images()->(c1,c2, ...),\nReturns a list of all
opened images" */
static std::vector<Canvas*> f_list_images(PyFuncContext& ctx){
  AppContext& app = ctx.app;
  return make_vector(up_to(app.GetCanvasCount()),
    [&app](const auto& index){
      return &app.GetCanvas(index);
    });
}


/* method: "set_active_image(image)\n
Activates (selects in a tab) the specified image." */
static void f_set_active_image(PyFuncContext& ctx, Canvas* canvas){
  ctx.app.SetActiveCanvas(canvas->GetId());
}

/* method: "set_layer(layer)\n
Select layer. 0=Raster, 1=Object" */
static void f_set_layer(PyFuncContext& ctx, int layer){
  if (!valid_layerstyle(layer)){
    throw ValueError(endline_sep(
      "Invalid value for layer.\nAcceptable values are:",
      space_sep(str_int(to_int(Layer::RASTER)), "- Raster layer"),
      space_sep(str_int(to_int(Layer::OBJECT)), "- Object layer")));
  }
  ctx.app.SetLayer(to_layerstyle(layer));
}

#include "generated/python/method-def/py-global-functions-methoddef.hh"

static PyObject* global_functions_new(PyTypeObject* type, PyObject*, PyObject*){
  globalFunctionsObject *self;
  self = (globalFunctionsObject*)type->tp_alloc(type, 0);
  self->ctx = nullptr;
  return (PyObject *)self;
}

static void global_functions_init(PyFuncContext&){
  // Prevent instantiation from Python,, since the expected context
  // can't be provided from there.
  throw TypeError(space_sep(GLOBAL_FUNCTIONS_NAME, "can not be instantiated."));
}

static PyObject* global_functions_repr(globalFunctionsObject*){
  return build_unicode(utf8_string(GLOBAL_FUNCTIONS_NAME));
}

PyTypeObject GlobalFunctionsType = {
  PyVarObject_HEAD_INIT(nullptr, 0)
  GLOBAL_FUNCTIONS_NAME, // tp_name
  sizeof(globalFunctionsObject), // tp_basicsize
  0, // tp_itemsize
  nullptr, // tp_dealloc
  nullptr, // tp_print
  nullptr, // tp_getattr
  nullptr, // tp_setattr
  nullptr, // reserved (formerly tp_compare)
  (reprfunc)global_functions_repr, // tp_repr
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
  global_functions_methods, // tp_methods
  nullptr, // tp_members
  nullptr, // tp_getset
  nullptr, // tp_base
  nullptr, // tp_dict
  nullptr, // tp_descr_get
  nullptr, // tp_descr_set
  0, // tp_dictoffset
  INIT_FORWARDER(global_functions_init), // tp_init
  nullptr, // tp_alloc
  global_functions_new, // tp_new
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

void add_global_functions(PyFuncContext& ctx, PyObject* module){
  add_type_object(module, GlobalFunctionsType, "GlobalFunctions");
  PyModule_AddObject(module, "fgl",
    create_python_object<globalFunctionsObject>(GlobalFunctionsType, ctx));
}


} // namespace
