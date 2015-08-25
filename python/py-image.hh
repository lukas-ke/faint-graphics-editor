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

#ifndef FAINT_PY_IMAGE_HH
#define FAINT_PY_IMAGE_HH
#include "python/py-include.hh"

namespace faint{

// Image type, with objects and such. Detached from Faint, for use in
// the Python-extension (and maybe eventually in Faint as well)
void add_type_Image(PyObject* module);

} // namespace

#endif
