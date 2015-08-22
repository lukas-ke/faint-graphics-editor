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

#ifndef FAINT_PY_INTERFACE_HH
#define FAINT_PY_INTERFACE_HH
#include <vector>
#include <string>
#include "text/utf8-string.hh"

namespace faint{

template<typename T> class BoundObject;
class Canvas;
class Command;
class Frame;
class FaintPyExc;
class Object;
class PythonContext;

void display_error_info(const FaintPyExc&, PythonContext&);

std::string get_python_version();

utf8_string python_get_command_name(const Canvas&);
void python_set_command_name(const Canvas&, const utf8_string&);

void run_python_str(const utf8_string&);

// Returns the names in the ifaint Python module __dict__.
std::vector<utf8_string> list_ifaint_names();

} // namespace

#endif
