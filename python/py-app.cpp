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

#include "app/get-app-context.hh"
#include "gui/transparency-style.hh"
#include "python/py-include.hh"
#include "python/mapped-type.hh"
#include "python/py-app.hh"
#include "python/py-ugly-forward.hh"
#include "text/formatting.hh"
#include "util/grid.hh"
#include "util-wx/file-path.hh"

namespace faint{

template<>
struct MappedType<AppContext&>{
  using PYTHON_TYPE = faintAppObject;

  static AppContext& GetCppObject(faintAppObject*){
    return get_app_context();
  }

  static bool Expired(faintAppObject*){
    // App can't (reasonably) go bad.
    return false;
  }

  static void ShowError(faintAppObject*){
    // App can't (reasonably) go bad.
  }

  static utf8_string DefaultRepr(faintAppObject*){
    return "FaintApp";
  }
};

/* method: "set_transparency_indicator(r,g,b)\n
Sets a color for indicating alpha transparency (instead of a checkered
pattern)" */
static void faintapp_set_transparency_indicator(AppContext& app,
  const ColRGB& color)
{
  app.SetTransparencyStyle(TransparencyStyle(color));
}

/* method: "set_checkered_transparency_indicator()\n
Use a checkered pattern to indicate transparency " */
static void faintapp_set_checkered_transparency_indicator(AppContext& app){
  app.SetTransparencyStyle(TransparencyStyle());
}

/* method: "get_checkered_transparency_indicator()->b\n
True if a checkered pattern is used to indicate transparency." */
static bool faintapp_get_checkered_transparency_indicator(AppContext& app){
return app.GetTransparencyStyle().IsCheckered(); }

/* method: "open([filepath1, filepath2,...])\n
Open the specified image files in new tabs." */
static void faintapp_open_files(AppContext& app,
  const std::vector<utf8_string>& pathStrings)
{
  FileList paths;
  for (const utf8_string& pathStr : pathStrings){
    if (!is_file_path(pathStr)){
      throw ValueError(space_sep(quoted(pathStr), "is not a file path"));
    }
    if (!is_absolute_path(pathStr)){
      throw ValueError(space_sep("Path", quoted(pathStr), "not absolute."));
    }

    paths.push_back(FilePath::FromAbsolute(pathStr));
  }
  if (paths.empty()){
    return;
  }

  app.Load(paths);
}

/* property: "gridcolor\n
The default color of grids used for new documents." */
struct faintapp_gridcolor{
  static Color Get(AppContext& app){
    return app.GetDefaultGrid().GetColor();
  }

  static void Set(AppContext& app, const Color& color){
    Grid grid(app.GetDefaultGrid());
    grid.SetColor(color);
    app.SetDefaultGrid(grid);
  }
};

/* property: "griddashed\n
Whether grids used for new documents are dashed or solid." */
struct faintapp_griddashed{
  static bool Get(AppContext& app){
    return app.GetDefaultGrid().Dashed();
  }

  static void Set(AppContext& app, bool dashed){
    Grid grid(app.GetDefaultGrid());
    grid.SetDashed(dashed);
    app.SetDefaultGrid(grid);
  }
};

#include "generated/python/method-def/py-app-methoddef.hh"

static void faintapp_init(faintAppObject&) {
}

static PyObject* faintapp_new(PyTypeObject* type, PyObject*, PyObject*){
  faintAppObject* self = (faintAppObject*)type->tp_alloc(type, 0);
  return (PyObject*)self;
}

static utf8_string faintapp_repr(AppContext&){
  return "FaintApp";
}

PyTypeObject FaintAppType = {
    PyVarObject_HEAD_INIT(nullptr, 0)
    "FaintApp", //tp_name
    sizeof(faintAppObject), // tp_basicsize
    0, // tp_itemsize
    nullptr, // tp_dealloc
    nullptr, // tp_print
    nullptr, // tp_getattr
    nullptr, // tp_setattr
    nullptr, // tp_compare
    REPR_FORWARDER(faintapp_repr), // tp_repr
    nullptr, // tp_as_number
    nullptr, // tp_as_sequence
    nullptr, // tp_as_mapping
    nullptr, // tp_hash
    nullptr, // tp_call
    nullptr, // tp_str
    nullptr, // tp_getattro
    nullptr, // tp_setattro
    nullptr, // tp_as_buffer
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags
    "The Faint Application Window", // tp_doc
    nullptr, // tp_traverse
    nullptr, // tp_clear
    nullptr, // tp_richcompare
    0, // tp_weaklistoffset
    nullptr, // tp_iter
    nullptr, // tp_iternext
    faintapp_methods, // tp_methods
    nullptr, // tp_members
    faintapp_getseters, // tp_getset
    nullptr, // tp_base
    nullptr, // tp_dict
    nullptr, // tp_descr_get
    nullptr, // tp_descr_set
    0, // tp_dictoffset
    INIT_FORWARDER(faintapp_init), // tp_init
    nullptr, // tp_alloc
    faintapp_new, // tp_new
    nullptr, // tp_free
    nullptr, // tp_is_gc
    nullptr, // tp_bases
    nullptr, // tp_mro
    nullptr, // tp_cache
    nullptr, // tp_subclasses
    nullptr, // tp_weaklist
    nullptr, // tp_del
    0, // tp_version_tag
    nullptr  // tp_finalize
};

} // namespace
