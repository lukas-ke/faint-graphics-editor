// -*- coding: us-ascii-unix -*-
// Copyright 2013 Lukas Kemmer
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
#ifdef _WIN32
// Py_DECREF uses while (0), which causes warning C4127: conditional
// expression is constant (Python2.7, Visual Studio 2010)
#pragma warning (disable : 4127)
#endif

namespace faint{

void py_xdecref(PyObject* o){
  if (o != nullptr){
    Py_DECREF(o);
  }
}

dumb_ptr<PyObject> borrowed(PyObject* o){
  return dumb_ptr<PyObject>(o);
}

} // namespace
