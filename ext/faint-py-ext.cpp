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

// Module definition for the Faint Python module.
// This is built to a shared library for use with a Python executable,
// it is not used within Faint.

namespace faint{

static PyMethodDef faintMethods[] = {
  {NULL, NULL, 0, NULL} // Sentinel
};

static struct PyModuleDef faintModule = {
   PyModuleDef_HEAD_INIT,
   "faint",   // Module name
   nullptr,  // Module docs
   -1, // Module "per-binterpreter state size"
   faintMethods
};

} // namespace

// Dynamic library entry point
PyMODINIT_FUNC PyInit_faint(){
  PyObject* module = PyModule_Create(&faint::faintModule);
  faint::add_type_object(module, faint::BitmapType, "Bitmap");
  return module;
}
