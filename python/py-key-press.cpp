// -*- coding: us-ascii-unix -*-
// Copyright 2013 Lukas Kemmer
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

#include <set>
#include <sstream>
#include "python/py-key-press.hh"
#include "python/py-include.hh"
#include "python/py-interface.hh"

namespace faint{

struct BindInfo_less{
  bool operator()(const BindInfo& b1, const BindInfo& b2) const {
    return b1.key < b2.key;
  }
};

void python_key_press(const KeyPress& key){
  std::stringstream ss;
  ss << "keypress(" << key.GetKeyCode() << "," << key.Modifiers().Raw() << ")";
  run_python_str(utf8_string(ss.str().c_str()));
}

std::vector<BindInfo> list_binds(){
  PyObject* module = PyImport_ImportModule("ifaint");
  assert(module != nullptr);
  PyObject* dict = PyModule_GetDict(module); // Borrowed ref
  PyObject* binds = PyDict_GetItemString(dict, "_binds"); // Borrowed ref
  PyObject* keys = PyDict_Keys(binds); // New ref
  int n = static_cast<int>(PySequence_Length(keys)); // Fixme: Check cast or change type
  std::set<BindInfo, BindInfo_less> bindList;

  for (int i = 0; i != n; i++){
    PyObject* tuple = PySequence_GetItem(keys, i); // New ref
    PyObject* objKey = PySequence_GetItem(tuple, 0); // New ref
    PyObject* objMod = PySequence_GetItem(tuple, 1); // New ref
    long keyCode = PyLong_AsLong(objKey);
    long mod = PyLong_AsLong(objMod);
    faint::py_xdecref(objKey);
    faint::py_xdecref(objMod);
    PyObject* obj_function = PyDict_GetItem(binds, tuple); // Borrowed ref
    PyObject* obj_functionName = PyObject_GetAttrString(obj_function, "__name__"); // New reference
    utf8_string functionName;
    if (PyUnicode_Check(obj_functionName)){
      PyObject* utf8 = PyUnicode_AsUTF8String(obj_functionName);
      if (utf8 != nullptr){
        functionName = PyBytes_AsString(utf8);
        faint::py_xdecref(utf8);
      }
    }
    if (obj_functionName != nullptr){
      py_xdecref(obj_functionName);
    }
    py_xdecref(tuple);
    KeyPress keyObj(Mod::Create(static_cast<int>(mod)), Key(static_cast<int>(keyCode))); // Fixme: Check cast or change type
    bindList.insert(BindInfo(keyObj, functionName, ""));
  }
  py_xdecref(keys);
  py_xdecref(module);
  return std::vector<BindInfo>(begin(bindList), end(bindList));
}

} // namespace
