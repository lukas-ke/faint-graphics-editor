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

#ifndef FAINT_PY_SHAPE_HH
#define FAINT_PY_SHAPE_HH
#include "python/py-include.hh"
#include "geo/rect.hh"
#include "util/settings.hh"
#include "util/optional.hh"

namespace faint{

// Object type with reference counting, with the object owned by
// Python, as opposed to py-something where the contained object
// is managed by the commands that added them and such.
void add_type_Shape(PyObject* module);

PyObject* create_Rect(const Rect&, const Optional<Settings>&);

} // namespace

#endif
