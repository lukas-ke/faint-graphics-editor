// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
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

#include "bitmap/bitmap.hh"
#include "python/py-function-error.hh"
#include "python/py-interface.hh"
#include "python/py-util.hh"
#include "text/formatting.hh"
#include "util/color-span.hh"

namespace faint{

PythonError::PythonError(PyObject* errorType, const utf8_string& error)
  : m_errorType(errorType),
    m_error(error)
{}

void PythonError::SetError() const{
  scoped_ref obj(build_unicode(m_error));
  PyErr_SetObject(m_errorType, obj.get());
}

IndexError::IndexError(const utf8_string& error)
  : PythonError(PyExc_IndexError, error)
{}

void throw_if_outside(const Index& index, const Index& bound){
  if (bound <= index){
    throw IndexError(space_sep("index out of range",
      no_sep("[0:", str_coder(bound), "].")));
  }
}

void throw_if_outside(const IntPoint& pt, const Bitmap& bmp){
  if (!point_in_bitmap(bmp, pt)){
    throw ValueError("Position outside image.");
  }
}


void throw_if_outside(const IntPoint& pt, const ColorSpan& span){
if (invalid_pixel_pos(pt, span)){
    throw ValueError("Position outside image.");
  }
}

ValueError::ValueError(const utf8_string& error)
  : PythonError(PyExc_ValueError, error)
{}

ValueError::ValueError(const utf8_string& error, int n)
  : PythonError(PyExc_ValueError, space_sep("Argument:", str_int(n + 1), error))
{}

TypeError::TypeError(const utf8_string& error)
  : PythonError(PyExc_TypeError, error)
{}

TypeError::TypeError(const TypeName& expectedType, int n)
  : PythonError(PyExc_TypeError,
    "Argument " + str_int(n + 1) + " must be a " +
    expectedType.Get())
{}

TypeError::TypeError(const TypeName& expectedType,
  const utf8_string& extraInfo,
  int n)
  : PythonError(PyExc_TypeError,
    space_sep("Argument", str_int(n + 1), "must be a",
      expectedType.Get(), no_sep(bracketed(extraInfo), ".")))
{}

OSError::OSError(const utf8_string& error)
  : PythonError(PyExc_OSError, error)
{}

MemoryError::MemoryError(const utf8_string& error)
  : PythonError(PyExc_MemoryError, error)
{}

PresetFunctionError::PresetFunctionError()
{}

void throw_insufficient_args_if(bool cond, const utf8_string& type){
  if (cond){
    throw TypeError("Too few arguments for parsing " + type);
  }
}

} // namespace
