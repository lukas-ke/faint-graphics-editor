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

#ifndef FAINT_PY_UTIL_HH
#define FAINT_PY_UTIL_HH
#include <vector>
#include "text/utf8-string.hh"
#include "util/common-fwd.hh"
#include "util/template-fwd.hh"
#include "python/py-exception.hh"

namespace faint{

// Attempts to decode the PyUnicode-object into an utf8-string.
// Sets a Python error and returns nothing on failure.
Optional<utf8_string> parse_py_unicode(PyObject*);

PyObject* build_unicode(const utf8_string&);
bool invalid_color(int r, int g, int b, int a=255);
bool invalid_pixel_pos(int x, int y, const Bitmap&);
bool invalid_pixel_pos(const IntPoint&, const Bitmap&);
bool parse_color(PyObject* sequence, Color&, bool allowAlpha=true);
bool parse_color_stop(PyObject*, ColorStop&);
std::vector<PathPt> parse_svg_path(const std::string& asciiStr);
utf8_string get_repr(const LinearGradient&);
utf8_string get_repr(const RadialGradient&);

PyObject* get_save_exception_type();
PyObject* get_load_exception_type();

bool py_error_occurred();
bool py_load_error_occurred();
bool py_save_error_occurred();
utf8_string py_error_string();

utf8_string stack_trace_str(PyObject* traceback);
FaintPyExc py_error_info();

} // namespace

#endif
