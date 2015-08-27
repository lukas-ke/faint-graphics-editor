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

#include <sstream>
#include "bitmap/bitmap.hh"
#include "bitmap/color-span.hh"
#include "python/py-function-error.hh"
#include "python/py-util.hh"
#include "text/formatting.hh"

namespace faint{

static utf8_string indefinite_article(const utf8_string& word){
  if (word.empty()){
    return "a";
  }
  const utf8_string vowels = "aAoOuUeEiIyY";
  return vowels.find(word.front()) == utf8_string::npos ?
    "a" : "an";
}

PythonError::PythonError(PyObject* errorType, const utf8_string& error)
  : m_errorType(errorType),
    m_error(error)
{}

void PythonError::SetError() const{
  scoped_ref obj(build_unicode(m_error));
  PyErr_SetObject(m_errorType, obj.get());
}

NotImplementedError::NotImplementedError(const utf8_string& error)
  : PythonError(PyExc_NotImplementedError, error)
{}

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

ValueError::ValueError(const utf8_string& error, Py_ssize_t n)
  : PythonError(PyExc_ValueError, space_sep("Argument:", str_ssize_t(n + 1), error)) // Fixme: Prevent overflow
{}

TypeError::TypeError(const utf8_string& error)
  : PythonError(PyExc_TypeError, error)
{}

TypeError::TypeError(const TypeName& expectedType, Py_ssize_t n)
  : PythonError(PyExc_TypeError,
      space_sep("Argument", str_ssize_t(n + 1),
        "must be", indefinite_article(expectedType.Get()),
        expectedType.Get()))
{}

TypeError::TypeError(const TypeName& expectedType,
  const utf8_string& extraInfo,
  Py_ssize_t n)
  : PythonError(PyExc_TypeError,
      space_sep("Argument", str_ssize_t(n + 1), "must be",
        indefinite_article(expectedType.Get()),
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
