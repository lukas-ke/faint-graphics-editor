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

#include "python/py-include.hh"
#include "python/py-add-type-object.hh"
#include "python/py-bitmap.hh"
#include "python/py-canvas.hh"
#include "python/py-color.hh"
#include "python/py-clipboard.hh"
#include "python/py-functions.hh"
#include "python/py-gradient.hh"
#include "python/py-exception-types.hh"
#include "python/py-frame.hh"
#include "python/py-image.hh"
#include "python/py-image-props.hh"
#include "python/py-pattern.hh"
#include "python/py-png.hh"
#include "python/py-settings.hh"
#include "python/py-shape.hh"
#include "python/py-something.hh"
#include "python/py-tri.hh"
#include "wx/image.h"

// Module definition for the Faint Python module. This is built to a
// shared library for use with a Python executable; it is not used
// within Faint.

namespace faint{

static struct PyModuleDef faintModule = {
   PyModuleDef_HEAD_INIT,
   "ifaint",   // Module name
   "Faint graphics module",  // Module docs
   -1, // Module "per-binterpreter state size"
   get_py_functions()
};

static void add_compatibility_types(PyObject* module){
  // These types are only useful in the Faint application, but are
  // added to the extension to avoid crashes on type-comparison in
  // py-parse.hh.
  //
  // Since they raise exceptions in their tp_init, it won't be
  // possible to instantiate them in the extension.
  add_type_object(module, CanvasType, "_Canvas");
  add_type_object(module, FrameType, "_Frame");

  add_type_object(module, SmthType, "_Something");
}

static PyObject* init_extension_module(){
  PyObject* ifaint = PyModule_Create(&faintModule);

  // Sub-modules
  add_clipboard_module(ifaint);
  add_png_module(ifaint);

  // Types
  add_type_Bitmap(ifaint);
  add_type_Color(ifaint);
  add_type_Image(ifaint);
  add_type_Pattern(ifaint);
  add_type_Settings(ifaint);
  add_type_Shape(ifaint);
  add_type_Tri(ifaint);

  // Some more types
  add_exception_types(ifaint);
  add_gradient_types(ifaint);
  add_image_props_types(ifaint);

  // And some I'd rather not have
  add_compatibility_types(ifaint);
  wxInitAllImageHandlers();
  return ifaint;
}

} // namespace

PyMODINIT_FUNC PyInit_ifaint(){
  // Dynamic library entry point
  return faint::init_extension_module();
}
