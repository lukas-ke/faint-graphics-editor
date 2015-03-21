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
#include "geo/limits.hh"
#include "python/py-key-press.hh"
#include "python/py-include.hh"
#include "python/py-interface.hh"
#include "python/py-util.hh"

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
  scoped_ref module(PyImport_ImportModule("ifaint"));
  assert(module != nullptr);
  PyObject* dict = PyModule_GetDict(module.get()); // Borrowed ref
  PyObject* binds = PyDict_GetItemString(dict, "_binds"); // Borrowed ref
  scoped_ref keys(PyDict_Keys(binds));
  auto n = PySequence_Length(keys.get());
  std::set<BindInfo, BindInfo_less> bindList;

  for (Py_ssize_t i = 0; i != n; i++){
    scoped_ref tuple(PySequence_GetItem(keys.get(), i));
    scoped_ref objKey(PySequence_GetItem(tuple.get(), 0));
    scoped_ref objMod(PySequence_GetItem(tuple.get(), 1));
    auto keyCode = as_int(objKey.get());
    assert(keyCode.IsSet());
    auto mod = as_int(objMod.get());
    assert(mod.IsSet());

    PyObject* obj_function = PyDict_GetItem(binds, tuple.get()); // Borrowed ref
    scoped_ref obj_functionName(PyObject_GetAttrString(obj_function, "__name__"));

    utf8_string functionName;
    if (PyUnicode_Check(obj_functionName.get())){
      scoped_ref utf8(PyUnicode_AsUTF8String(obj_functionName.get()));
      if (utf8 != nullptr){
        functionName = PyBytes_AsString(utf8.get());
      }
    }

    KeyPress keyObj(Mod::Create(mod.Get()), Key(keyCode.Get()));
    bindList.insert(BindInfo(keyObj, functionName, ""));
  }
  return std::vector<BindInfo>(begin(bindList), end(bindList));
}

} // namespace
