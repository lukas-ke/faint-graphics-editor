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
#include "python/py-png.hh"
#include "python/py-settings.hh"
#include "python/py-functions.hh"
#include "python/py-frame-props.hh"
#include "python/py-image-props.hh"

// Module definition for the Faint Python module. This is built to a
// shared library for use with a Python executable; it is not used
// within Faint.

namespace faint{

static struct PyModuleDef faintModule = {
   PyModuleDef_HEAD_INIT,
   "faint",   // Module name
   "Faint graphics module",  // Module docs
   -1, // Module "per-binterpreter state size"
   get_py_functions()
};

} // namespace

// Dynamic library entry point
PyMODINIT_FUNC PyInit_faint(){
  PyObject* module = PyModule_Create(&faint::faintModule);
  faint::add_type_object(module, faint::BitmapType, "Bitmap");
  faint::add_type_object(module, faint::SettingsType, "Settings");
  faint::add_png_module(module);
  faint::add_type_ImageProps(module);
  faint::add_type_FrameProps(module);

  return module;
}
