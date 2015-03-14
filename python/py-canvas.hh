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

#ifndef FAINT_PY_CANVAS_HH
#define FAINT_PY_CANVAS_HH
#include "python/mapped-type.hh"
#include "util/id-types.hh"

namespace faint{

class Canvas;
class IntPoint;

extern PyTypeObject CanvasType;

struct canvasObject{
  PyObject_HEAD
  Canvas* canvas;
  CanvasId id;
};

bool canvas_ok(const CanvasId&);
PyObject* pythoned(Canvas&);
bool contains_pos(const Canvas&, const IntPoint&);

} // namespace

#endif
