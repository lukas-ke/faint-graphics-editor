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

#ifndef FAINT_PY_SOMETHING_HH
#define FAINT_PY_SOMETHING_HH
#include "bound-object.hh"
#include "util/id-types.hh"

namespace faint{

class Object;
class Canvas;

void add_type_Something(PyObject* module);

bool is_Something(PyObject*);

Object* Something_as_Object(PyObject*);
BoundObject<Object> Something_as_BoundObject(PyObject*);

PyObject* pythoned(Object*, PyFuncContext& ctx, Canvas*, const FrameId&);

} // namespace

#endif
