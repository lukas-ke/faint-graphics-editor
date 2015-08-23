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

#include "python/py-png.hh"

namespace faint{

static struct PyModuleDef pngModule = {
  PyModuleDef_HEAD_INIT,
  "ifaint_png",
  "PNG constants, for use with the write_png function.",
  -1, // m_size
  nullptr, // m_methods
  nullptr, // m_reload
  nullptr, // m_traverse
  nullptr, // m_clear
  nullptr, // m_free
};

static PyObject* create_png_module(){
  PyObject* module = PyModule_Create(&pngModule);
  PyModule_AddIntConstant(module, "RGB", 0);
  PyModule_AddIntConstant(module, "RGB_ALPHA", 1);
  PyModule_AddIntConstant(module, "GRAY", 2);
  PyModule_AddIntConstant(module, "GRAY_ALPHA", 3);
  return module;
}

void add_png_module(PyObject* parentModule){
  PyModule_AddObject(parentModule, "png", create_png_module());
}

} // namespace
