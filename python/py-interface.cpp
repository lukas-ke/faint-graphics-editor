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

#include "app/frame.hh"
#include "app/get-python-context.hh"
#include "python/bound-object.hh"
#include "python/py-exception.hh"
#include "python/py-interface.hh"
#include "python/py-include.hh"
#include "python/py-func-context.hh"
#include "text/char-constants.hh"
#include "text/formatting.hh"

namespace faint{

std::string get_python_version(){
  return Py_GetVersion();
}

void run_python_str(const utf8_string& cmd, PythonContext& ctx){
  utf8_string cmd_silent("push_silent(\"" + cmd + "\")");
  PyRun_SimpleString(cmd_silent.c_str());
  // run_python_str is for invoking Python from the C++-code, not the
  // interpreter - this case also requires calling EvalDone.
  ctx.EvalDone();
}

static std::vector<utf8_string> names_in_sequence(PyObject* seq){
  std::vector<utf8_string> names;
  const Py_ssize_t n = PySequence_Length(seq);
  for (Py_ssize_t i = 0; i != n; i++){
    scoped_ref name(PySequence_GetItem(seq, i));
    scoped_ref utf8(PyUnicode_AsUTF8String(name.get()));
    assert(utf8 != nullptr);
    char* str = PyBytes_AsString(utf8.get()); // Should not be deallocated
    names.emplace_back(str);
  }
  return names;
}

std::vector<utf8_string> list_ifaint_names(){
  scoped_ref module(PyImport_ImportModule("ifaint"));
  assert(module != nullptr);

  auto dict = borrowed(PyModule_GetDict(module.get()));
  assert(dict != nullptr);

  scoped_ref keys(PyDict_Keys(dict.get()));
  assert(keys != nullptr);

  return names_in_sequence(keys.get());
}

} // namespace
