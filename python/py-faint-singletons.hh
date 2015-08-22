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

#ifndef FAINT_PY_FAINT_SINGLETONS_HH
#define FAINT_PY_FAINT_SINGLETONS_HH

namespace faint{

/** Methods for modifying the active tool settings.*/
void add_ActiveSettings(AppContext&, PyObject* module);

/** Application level methods, whatever that means... :) */
void add_App(PyFuncContext&, PyObject* module);

/** Methods for showing Faint dialogs. */
void add_dialog_functions(PyFuncContext&, PyObject* module);

void add_global_functions(PyFuncContext&, PyObject* module);

void add_Interpreter(AppContext&, PyObject* module);

void add_Palette(AppContext&, PyObject* module);

void add_Window(AppContext&, PyObject* module);

} // namespace

#endif
