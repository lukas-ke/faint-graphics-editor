// -*- coding: us-ascii-unix -*-
// Copyright 2016 Lukas Kemmer
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

#ifndef FAINT_PY_COLOR_HH
#define FAINT_PY_COLORHH
#include "python/py-include.hh"

namespace faint{

class Color;

// Add Color object to Python-module
void add_type_Color(PyObject* module);

// Return the C++-color if the object is a Python Color-object,
// otherwise nullptr.
Color* as_Color(PyObject*);

// Creates a new Python color object
PyObject* pythoned(const Color&);

} // namespace

#endif
