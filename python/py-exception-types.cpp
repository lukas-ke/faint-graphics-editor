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

#include "python/py-exception-types.hh"

namespace faint{

static PyObject* get_load_exception_type(){
  static PyObject* faintPyLoadError = PyErr_NewException("ifaint.LoadError",
    nullptr, nullptr);
  return faintPyLoadError;
}

static PyObject* get_save_exception_type(){
  static PyObject* faintPySaveError = PyErr_NewException("ifaint.SaveError",
    nullptr, nullptr);
  return faintPySaveError;
}

bool py_load_error_occurred(){
  PyObject* errType = PyErr_Occurred();
  if (errType == nullptr){
    return false;
  }
  PyObject* loadError = get_load_exception_type();
  int result = PyErr_GivenExceptionMatches(errType, loadError);
  return result != 0;
}

bool py_save_error_occurred(){
  PyObject* errType = PyErr_Occurred();
  if (errType == nullptr){
    return false;
  }
  PyObject* saveError = get_save_exception_type();
  int result = PyErr_GivenExceptionMatches(errType, saveError);
  return result != 0;
}

void add_exception_types(PyObject* module){
  PyModule_AddObject(module, "LoadError", get_load_exception_type());
  PyModule_AddObject(module, "SaveError", get_save_exception_type());
}

} // namespace
