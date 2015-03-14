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

#ifndef FAINT_PY_FUNCTION_ERROR_HH
#define FAINT_PY_FUNCTION_ERROR_HH
#include "python/py-include.hh"
#include "text/utf8-string.hh"
#include "util/distinct.hh"
#include "util/index.hh"

namespace faint{

// Exceptions for errors in Python interface functions. Caught by the
// function forwarding in py-ugly-forward.hh.

class Bitmap;
class IntPoint;
class ColorSpan;

class PythonError{
public:
  PythonError(PyObject*, const utf8_string&);
  // Sets the error using PyErr_...
  void SetError() const;
  virtual ~PythonError() = default;
private:
  PyObject* m_errorType;
  utf8_string m_error;
};

class IndexError : public PythonError{
public:
  IndexError(const utf8_string&);
};

void throw_if_outside(const Index& index, const Index& bound);
void throw_if_outside(const IntPoint&, const Bitmap&);
void throw_if_outside(const IntPoint&, const ColorSpan&);

class ValueError : public PythonError{
public:
  ValueError(const utf8_string&);
  ValueError(const utf8_string&, int n);
};

class category_type_error;
using TypeName = Distinct<utf8_string, category_type_error, 0>;

class TypeError : public PythonError{
public:
  TypeError(const utf8_string&);
  TypeError(const TypeName& expectedType, int argNum);
  TypeError(const TypeName& expectedType, const utf8_string& extraInfo,
    int argNum);
};

class OSError : public PythonError{
public:
  OSError(const utf8_string&);
};

class MemoryError : public PythonError{
public:
  MemoryError(const utf8_string&);
};

class PresetFunctionError{
  // Exception for errors in Python interface functions. Should be
  // thrown if a specific error has already been set with
  // PyErr_SetString. Caught by the function forwarding in
  // py-ugly-forward.hh (which will not set any Python-error).
public:
  PresetFunctionError();

private:
};

void throw_insufficient_args_if(bool, const utf8_string& type);

} // namespace

#endif
