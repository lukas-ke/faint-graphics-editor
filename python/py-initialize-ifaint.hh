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

#ifndef FAINT_PY_INITIALIZE_IFAINT_HH
#define FAINT_PY_INITIALIZE_IFAINT_HH
#include "text/utf8-string.hh"
#include "util/template-fwd.hh"

namespace faint{

class FaintPyExc;
class FilePath;
class PythonContext;

// Initializes the ifaint module.
// The string argument will be available as ifaint.cmd_arg.
// Returns true on success.
bool init_python(const utf8_string& arg);

Optional<FaintPyExc> run_python_file(const FilePath&);

// Runs the user configuration. Returns true if loaded without error
bool run_python_user_config(PythonContext&);

} // namespace

#endif
