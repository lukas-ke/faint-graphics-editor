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
#include "text/char-constants.hh"
#include "text/formatting.hh"

namespace faint{

std::string get_python_version(){
  return Py_GetVersion();
}

utf8_string format_error_info(const FaintPyExc& info){
  utf8_string errStr = no_sep(info.stackTrace);
  if (info.syntaxErrorInfo.IsSet()){
    FaintPySyntaxError syntaxError = info.syntaxErrorInfo.Get();
    errStr += "\n";
    errStr += ("  File " + quoted(syntaxError.file) + ", line " +
      str_int(syntaxError.line) + "\n");
    errStr += ("    " + syntaxError.code);
    if (syntaxError.col > 0){
      if (syntaxError.code.str().back() != '\n'){
        errStr += "\n";
      }

      // Put a '^'-under the start of the syntax error
      errStr += ("    " + utf8_string(to_size_t(syntaxError.col - 1), space) +
        "^\n");
    }
  }
  errStr += info.type + ": " + info.message + "\n";
  errStr += "\n";
  return errStr;
}

void python_print(const utf8_string& text){
  get_python_context().Print(text);
}

void python_queue_refresh(Canvas& canvas){
  get_python_context().QueueRefresh(&canvas);
}

void python_run_command(Canvas& canvas, Command* cmd){
  if (cmd != nullptr){
    get_python_context().RunCommand(&canvas, cmd);
  }
}

void python_run_command(const BoundObject<Object>& obj, Command* cmd){
  if (cmd != nullptr){
    get_python_context().RunCommand(obj.canvas, cmd, obj.frameId);
  }
}

void python_run_command(const Frame& frame, Command* cmd){
  if (cmd != nullptr){
    get_python_context().RunCommand(frame.canvas, cmd, frame.frameId);
  }
}

utf8_string python_get_command_name(const Canvas& canvas){
  return get_python_context().GetCommandName(&canvas);
}

void python_set_command_name(const Canvas& canvas, const utf8_string& name){
  get_python_context().SetCommandName(&canvas, name);
}

void run_python_str(const utf8_string& cmd){
  utf8_string cmd_silent("push_silent(\"" + cmd + "\")");
  PyRun_SimpleString(cmd_silent.c_str());
  // run_python_str is for invoking Python from the C++-code, not the
  // interpreter - this case also requires calling EvalDone.
  get_python_context().EvalDone();
}

std::vector<utf8_string> list_ifaint_names(){
  scoped_ref module(PyImport_ImportModule("ifaint"));
  assert(module != nullptr);

  auto dict = borrowed(PyModule_GetDict(module.get()));
  assert(dict != nullptr);

  scoped_ref keys(PyDict_Keys(dict.get()));
  assert(keys != nullptr);

  std::vector<utf8_string> names;
  const int n = static_cast<int>(PySequence_Length(keys.get())); // Fixme: Check cast or change type
  for (int i = 0; i != n; i++){
    scoped_ref name(PySequence_GetItem(keys.get(), i));
    scoped_ref utf8(PyUnicode_AsUTF8String(name.get()));
    assert(utf8 != nullptr);
    char* str = PyBytes_AsString(utf8.get()); // Should not be deallocated
    names.emplace_back(str);
  }
  return names;
}

} // namespace
