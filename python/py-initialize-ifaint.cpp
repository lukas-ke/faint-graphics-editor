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

#include <fstream>
#include "python/py-include.hh"
#include "python/py-initialize-ifaint.hh"
#include "python/py-bitmap.hh"
#include "python/py-canvas.hh"
#include "python/py-faint-singletons.hh"
#include "python/py-frame.hh"
#include "python/py-functions.hh"
#include "python/py-gradient.hh"
#include "python/py-grid.hh"
#include "python/py-image-props.hh"
#include "python/py-pattern.hh"
#include "python/py-png.hh"
#include "python/py-settings.hh"
#include "python/py-something.hh"
#include "python/py-shape.hh"
#include "python/py-util.hh" // build_unicode
#include "python/py-tri.hh"
#include "python/py-exception-types.hh"
#include "python/python-context.hh"
#include "python/py-add-type-object.hh"
#include "python/py-clipboard.hh"
#include "util/paint-map.hh"
#include "util-wx/key-codes.hh"
#include "util-wx/file-path.hh"
#include "util-wx/file-path-util.hh"
#include "text/formatting.hh"

namespace faint{

static void add_faint_types(PyObject* module){
  add_type_Bitmap(module);
  add_type_Grid(module);
  add_type_Pattern(module);
  add_type_Settings(module);
  add_type_Shape(module);
  add_type_Tri(module);

  add_gradient_types(module);
  add_image_props_types(module);

  add_type_object(module, CanvasType, "Canvas");
  add_type_object(module, FrameType, "Frame");
  add_type_object(module, SmthType, "Something");

  PyObject* binds = PyDict_New();
  PyModule_AddObject(module, "_binds", binds);
  add_exception_types(module);
}

static struct PyModuleDef keyModule = {
  PyModuleDef_HEAD_INIT,
  "ifaint_key",
  "Key identifiers",
  -1, // m_size
  nullptr, // m_methods
  nullptr, // m_reload
  nullptr, // m_traverse
  nullptr, // m_clear
  nullptr, // m_free
};

static PyObject* create_key_module(){
  PyObject* module = PyModule_Create(&keyModule);
  PyModule_AddIntConstant(module, "arrow_down", key::down);
  PyModule_AddIntConstant(module, "arrow_left", key::left);
  PyModule_AddIntConstant(module, "arrow_right", key::right);
  PyModule_AddIntConstant(module, "arrow_up", key::up);
  PyModule_AddIntConstant(module, "asterisk", key::asterisk);
  PyModule_AddIntConstant(module, "backspace", key::back);
  PyModule_AddIntConstant(module, "delete", key::del);
  PyModule_AddIntConstant(module, "end", key::end);
  PyModule_AddIntConstant(module, "f1", key::F1);
  PyModule_AddIntConstant(module, "f2", key::F2);
  PyModule_AddIntConstant(module, "f3", key::F3);
  PyModule_AddIntConstant(module, "f4", key::F4);
  PyModule_AddIntConstant(module, "f5", key::F5);
  PyModule_AddIntConstant(module, "f6", key::F6);
  PyModule_AddIntConstant(module, "f7", key::F7);
  PyModule_AddIntConstant(module, "f8", key::F8);
  PyModule_AddIntConstant(module, "f9", key::F9);
  PyModule_AddIntConstant(module, "f10", key::F10);
  PyModule_AddIntConstant(module, "f11", key::F11);
  PyModule_AddIntConstant(module, "f12", key::F12);
  PyModule_AddIntConstant(module, "home", key::home);
  PyModule_AddIntConstant(module, "num_minus", key::num_minus);
  PyModule_AddIntConstant(module, "num_plus", key::num_plus);
  PyModule_AddIntConstant(module, "paragraph", key::paragraph);
  PyModule_AddIntConstant(module, "pgdn", key::pgdn);
  PyModule_AddIntConstant(module, "pgup", key::pgup);
  PyModule_AddIntConstant(module, "space", key::space);
  return module;
}

static struct PyModuleDef modifierModule = {
  PyModuleDef_HEAD_INIT,
  "ifaint_mod",
  "Key modifier identifiers",
  -1, // m_size
  nullptr, // m_methods
  nullptr, // m_reload
  nullptr, // m_traverse
  nullptr, // m_clear
  nullptr, // m_free
};

static PyObject* create_modifier_module(){
  PyObject* module = PyModule_Create(&modifierModule);
  PyModule_AddIntConstant(module, "alt", Alt.Raw());
  PyModule_AddIntConstant(module, "shift", Shift.Raw());
  PyModule_AddIntConstant(module, "ctrl", Ctrl.Raw());
  return module;
}

static struct PyModuleDef faintInterfaceModule = {
  PyModuleDef_HEAD_INIT,
  "ifaint",
  "ifaint\n\nThe built in functions and classes for faint-graphics-editor.\n",
  -1, // m_size
  get_py_functions(), // m_methods
  nullptr, // m_reload
  nullptr, // m_traverse
  nullptr, // m_clear
  nullptr, // m_free
};

static PyObject* ifaintError;

PyMODINIT_FUNC PyInit_ifaint(){
  PyObject* module_ifaint = PyModule_Create(&faintInterfaceModule);
  assert(module_ifaint != nullptr);

  add_faint_types(module_ifaint);

  ifaintError = PyErr_NewException("ifaint.error", nullptr, nullptr);
  Py_INCREF(ifaintError);

  PyModule_AddObject(module_ifaint, "error", ifaintError);
  PyModule_AddObject(module_ifaint, "key", create_key_module());
  PyModule_AddObject(module_ifaint, "mod", create_modifier_module());

  add_clipboard_module(module_ifaint);
  add_png_module(module_ifaint);

  return module_ifaint;
}

static void add_to_python_path(const DirPath& dirPath){
  scoped_ref sys(PyImport_ImportModule("sys"));
  assert(sys != nullptr);

  auto dict = borrowed(PyModule_GetDict(sys.get()));
  assert(dict != nullptr);

  auto path = borrowed(PyDict_GetItemString(dict.get(), "path"));
  assert(path != nullptr);

  // Prepend the path so that the "faint" py module takes precedence
  // over any namesakes.
  int result = PyList_Insert(path.get(), 0, build_unicode(dirPath.Str()));
  assert(result == 0);
}

static void run_envsetup(const FilePath& path){
  assert(exists(path));
  auto err = run_python_file(path);
  assert(err.NotSet());
}

static void add_python_singletons(PyObject* ifaint, PyFuncContext& ctx){
  // These objects require "context", are rather tightly coupled
  // to Faint and can not be created from the Python side.
  add_App(ctx, ifaint);
  add_global_functions(ctx, ifaint);
  add_dialog_functions(ctx, ifaint);
  add_ActiveSettings(ctx.app, ifaint);
  add_Palette(ctx.app, ifaint);
  add_Window(ctx.app, ifaint);
  add_Interpreter(ctx.app, ifaint);
}

bool init_python(const utf8_string& arg, PyFuncContext& ctx){
  PyImport_AppendInittab("ifaint", PyInit_ifaint);
  Py_Initialize();

  scoped_ref ifaint(PyImport_ImportModule("ifaint"));
  add_python_singletons(ifaint.get(), ctx);

  DirPath dataDir = get_data_dir();

  // Add the py-dir to path, so that Faint:s .py-files (in py/faint/)
  // are found from envsetup and the user ini.
  add_to_python_path(dataDir.SubDir("py"));

  if (!arg.empty()){
    auto dict = borrowed(PyModule_GetDict(ifaint.get()));
    PyDict_SetItemString(dict.get(), CMDLINE_ARGUMENT_NAME,
      build_unicode(arg));
  }

  run_envsetup(dataDir.SubDir("py").SubDir("core").File("envsetup.py"));

  return true;
}

void display_error_info(const FaintPyExc& info, PythonContext& python){
  python.IntFaintPrint(format_error_info(info));
}

Optional<FaintPyExc> run_python_file(const FilePath& path){
  std::ifstream f(iostream_friendly(path));
    if (!f.good()){
      FaintPyExc err;
      err.type = "OSError";
      err.message = "Failed opening " + path.Str();
      return option(err);
    }

    std::string text;
    std::string line;
    while (std::getline(f, line)){
      text += line + "\n";
    }

    scoped_ref module(PyImport_ImportModule("__main__"));
    assert(module != nullptr);
    auto dict = borrowed(PyModule_GetDict(module.get()));
    assert(dict != nullptr);
    scoped_ref obj(PyRun_String(text.c_str(), Py_file_input, dict.get(),
      dict.get()));
    if (obj == nullptr){
      return option(py_error_info());
    }
    return no_option();
}

static void write_python_user_config(const FilePath& dstPath,
  PythonContext& python)
{
  // Rewriting the ini file instead of copying it should give
  // the os-correct eol markers

  Optional<FilePath> srcPath = make_absolute_file_path(
    get_data_dir().Str() + "/py/core/default_ini.py");
  assert(srcPath.IsSet());

  std::ifstream defaultIni(iostream_friendly(srcPath.Get()));
  if (!defaultIni.good()){
    python.IntFaintPrint("Failed opening standard ini");
    return;
  }
  std::ofstream userIni(iostream_friendly(dstPath));

  std::string line;
  while (std::getline(defaultIni, line)){
    userIni << line << std::endl;
  }
}

bool run_python_user_config(PythonContext& python){
  // Execute the user's ini-script
  FilePath configPath(get_user_config_file_path());
  if (!exists(configPath)){
    // Recreate the config file from the default
    write_python_user_config(configPath, python);
  }

  if (exists(configPath)){
    Optional<FaintPyExc> err = run_python_file(configPath);
    if (err.IsSet()){
      python.IntFaintPrint("\n");
      python.IntFaintPrint(space_sep("Error in personal config file",
          bracketed(quoted(configPath.Str()))) + ":\n");

      display_error_info(err.Get(), python);
      return false;
    }
    else{
      python.IntFaintPrint(space_sep("Executed personal config file at",
        quoted(configPath.Str())) + "\n");
      return true;
    }
  }
  else {
    utf8_string userIniInfo(
      space_sep("Personal config file not found at",
        quoted(configPath.Str())));
    python.IntFaintPrint(userIniInfo + "\n");
    return false;
  }
  return true;
}

} // namespace
