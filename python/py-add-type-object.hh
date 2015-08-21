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

#ifndef FAINT_ADD_TYPE_OBJECT
#define FAINT_ADD_TYPE_OBJECT

namespace faint {

template<typename T>
void add_type_object(PyObject* module, T& type, const char* name){
  type.tp_new = PyType_GenericNew;
  int result = PyType_Ready(&type);
  assert(result >= 0);
  Py_INCREF(&type);
  PyModule_AddObject(module, name, (PyObject*)&type);
}

template<typename OBJ_TYPE, typename TYPE_TYPE, typename CTX_T>
PyObject* create_python_object(TYPE_TYPE& type, CTX_T& ctx){
  auto* py_obj = (OBJ_TYPE*)type.tp_alloc(&type, 0);
  py_obj->ctx = &ctx;
  return (PyObject*)py_obj;
}

} // namespace

#endif
