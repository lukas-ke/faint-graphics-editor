// -*- coding: us-ascii-unix -*-
// Copyright 2012 Lukas Kemmer
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

#ifndef FAINT_PY_INCLUDE_HH
#define FAINT_PY_INCLUDE_HH
#include "util/dumb-ptr.hh"

#ifdef _WIN32

#pragma warning(push)
// Warnings emitted by Python header since 3.4
#pragma warning(disable:4510) // default constructor could not be generated
#pragma warning(disable:4512) // 'class' : assignment operator could not be generated
#pragma warning(disable:4610) // 'class' can never be instantiated - user-defined constructor required
#endif

// On Windows, debug python (python##_d.lib) would be linked if _DEBUG
// is defined temporarily undefine to allow linking release Python
// with a debug build of Faint.
#ifdef _DEBUG
#define FAINT_UNDEFFED_DEBUG
#undef _DEBUG
#endif

#include <Python.h>

#ifdef _WIN32
#pragma warning(pop)
#endif
// Redefine _DEBUG
#ifdef  FAINT_UNDEFFED_DEBUG
#define _DEBUG
#undef FAINT_UNDEFFED_DEBUG
#endif

namespace faint{

// Custom variant of Py_XDECREF, to avoid visual studio warning
// C4127: conditional expression is constant
void py_xdecref(PyObject*);

struct Decreffer{
  void operator()(PyObject* obj){
    py_xdecref(obj);
  }
};

template<typename T>
struct CastDecreffer{
  void operator()(T* obj){
    py_xdecref((PyObject*)obj);
  }
};

dumb_ptr<PyObject> borrowed(PyObject*);

// Pointer to PyObject* which decrements the objects reference count
// when it goes out of scope.
using scoped_ref = std::unique_ptr<PyObject, Decreffer>;

// Pointer to specific types (which must be PyObject*:s)
//
template<typename T>
using typed_scoped_ref = std::unique_ptr<T, CastDecreffer<T>>;

// Increments the ref-count of the argument and returns a new reference
scoped_ref new_ref(PyObject*);

template<typename T>
constexpr bool managed(const T&){
  return
    std::is_same_v<typename T::deleter_type, Decreffer> ||
    std::is_same_v<typename T::deleter_type, CastDecreffer<typename T::element_type>>;
}

const int init_ok = 0;
const int init_fail = -1;

template<typename T>
PyObject* py_rich_compare(const T& lhs, const T& rhs, int op){
  if (lhs < rhs){
    if (op == Py_LT || op == Py_LE || op == Py_NE){
      Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
  }
  else if (lhs == rhs){
    if (op == Py_EQ || op == Py_LE || op == Py_GE){
      Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
  }
  else{
    assert(lhs > rhs);
    if (op == Py_GT || op == Py_GE || op == Py_NE){
      Py_RETURN_TRUE;
    }
    Py_RETURN_FALSE;
  }
}

} // namespace

#endif
