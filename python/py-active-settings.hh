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

#ifndef FAINT_PY_ACTIVE_SETTINGS_HH
#define FAINT_PY_ACTIVE_SETTINGS_HH
#include "python/mapped-type.hh"
#include "python/py-include.hh"
#include "python/py-func-context.hh"

namespace faint{

/** Add Python-object with methods for modifying the active tool
settings. The object can only be created from the C++-code. */
void add_ActiveSettings(PyFuncContext&, PyObject* module);

} // namespace

#endif
