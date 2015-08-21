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

#include "app/app-context.hh"
#include "app/canvas.hh"
#include "formats/format.hh"
#include "gui/transparency-style.hh"
#include "text/formatting.hh"
#include "tools/tool-id.hh"
#include "util-wx/font.hh"
#include "util-wx/file-path.hh"
#include "util/enum-util.hh"
#include "util/grid.hh"
#include "util/make-vector.hh"
#include "util/setting-id.hh"
#include "python/py-include.hh"
#include "python/mapped-type.hh"
#include "python/py-add-type-object.hh"
#include "python/py-format.hh"
#include "python/py-ugly-forward.hh"

namespace faint{

class AppContext;

extern PyTypeObject FaintAppType;

struct faintAppObject {
  PyObject_HEAD
  AppContext* ctx;
};

template<>
struct MappedType<AppContext&>{
  using PYTHON_TYPE = faintAppObject;

  static AppContext& GetCppObject(faintAppObject* obj){
    return *obj->ctx;
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

/* method: "add_format(load_callback, save_callback, name, extension)\n
Add a file format for loading and/or saving.\n\n
Either load_callback or save_callback can be None.\n
name is the name of the file format,
extension is the file extension, used to identify the format.\n\n
The load_callback receives a filename and an ImageProps object:\n
my_load_callback(filename, imageProps)\n\nThe save_callback receives a
target filename and a Canvas:\n
my_save_callback(filename, canvas)" */
static void faintapp_add_format(AppContext& app, PyObject* args){
  PyObject* loader;
  PyObject* saver;
  char* name;
  char* extension;
  if (!PyArg_ParseTuple(args, "OOss", &loader, &saver, &name, &extension)){
    throw PresetFunctionError();
  }

  if (loader == Py_None){
    loader = nullptr;
  }
  else if (!PyCallable_Check(loader)){
    throw TypeError("Loader must be a callback or None:\n"
      "add_format(loader, saver, name, extension)");
  }
  else {
    Py_INCREF(loader);
  }

  if (saver == Py_None){
    saver = nullptr;
  }
  else if (!PyCallable_Check(saver)){
    throw TypeError("Saver must be a callback or None:\n"
      "add_format(loader, saver, name, extension)");
  }
  else{
    Py_INCREF(saver);
  }

  if (loader == nullptr && saver == nullptr){
    throw ValueError("saver and loader are both None");
  }
  PyFileFormat* f = new PyFileFormat(load_callback_t(loader),
    save_callback_t(saver),
    label_t(utf8_string(name)),
    FileExtension(extension));
  app.AddFormat(f);
}

/* method: "close()\n
Close the canvas." */
static void faintapp_close(AppContext& app, Canvas* canvas){
  app.Close(*canvas);
}


/* method: "list_fonts()->[fontname1, ...]\n
Returns a list of the available font names." */
static std::vector<utf8_string> faintapp_list_fonts(AppContext&){
  return available_font_facenames();
}

/* method: "list_formats()->[file_format, ...]\n
Returns a list of the available file formats as tuples with
(format_name, default_extension)." */
static auto faintapp_list_formats(AppContext& app){
  return make_vector(app.GetFormats(),
    [](const Format* f){
      return std::make_pair(f->GetLabel(), f->GetDefaultExtension().Str());
    });
}

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

/* method: "quit()\nExit faint" */
static void faintapp_quit(AppContext& app){
  app.Quit(); // Fixme: Crashes. :)
}

/* method: "swap_colors()\nSwaps the foreground and background colors." */
static void faintapp_swap_colors(AppContext& app){
  Paint fg(app.Get(ts_Fg));
  Paint bg(app.Get(ts_Bg));
  app.Set(ts_Fg, bg);
  app.Set(ts_Bg, fg);
}

/* method: "tool([i])\n
Selects the tool with the specified id, or returns the current id if
none specified." */
static Optional<int> faintapp_tool(AppContext& app, const Optional<int>& maybeId){
  if (maybeId.NotSet()){
    // Return the currently selected tool identifier
    return option(to_int(app.GetToolId()));
  }

  int id = maybeId.Get();
  if (!within_enum<ToolId>(id)){
    throw ValueError(space_sep(
      "Invalid tool identifier. Valid tool identifiers are",
      str_int(enum_min_value<ToolId>()), "to", str_int(enum_max_value<ToolId>())));
  }

  app.SelectTool(to_tool_id(id));

  // Returns nothing when selecting a tool.
  return no_option();
}

/* method: "tool_selection(AppContext& app)\nSelects the raster selection tool." */
static void faintapp_tool_selection(AppContext& app){
  app.SelectTool(ToolId::SELECTION);
}

/* method: "tool_raster_selection(AppContext& app)\nSelects the raster selection tool." */
static void faintapp_tool_raster_selection(AppContext& app){
  app.SelectTool(ToolId::SELECTION);
  app.SetLayer(Layer::RASTER);
}

/* method: "tool_object_selection(AppContext& app)\nSelects the object selection tool." */
static void faintapp_tool_object_selection(AppContext& app){
  app.SelectTool(ToolId::SELECTION);
  app.SetLayer(Layer::OBJECT);
}

/* method: "tool_pen(AppContext& app)\nSelects the pen tool." */
static void faintapp_tool_pen(AppContext& app){
  app.SelectTool(ToolId::PEN);
}

/* method: "tool_brush(AppContext& app)\nSelects the brush tool." */
static void faintapp_tool_brush(AppContext& app){
  app.SelectTool(ToolId::BRUSH);
}

/* method: "tool_picker(AppContext& app)\nSelects the picker tool." */
static void faintapp_tool_picker(AppContext& app){
  app.SelectTool(ToolId::PICKER);
}

/* method: "tool_path(AppContext& app)\nSelects the path tool." */
static void faintapp_tool_path(AppContext& app){
  app.SelectTool(ToolId::PATH);
}

/* method: "tool_level(AppContext& app)\nSelects the level tool." */
static void faintapp_tool_level(AppContext& app){
  app.SelectTool(ToolId::LEVEL);
}

/* method: "tool_line(AppContext& app)\nSelects the line tool." */
static void faintapp_tool_line(AppContext& app){
  app.SelectTool(ToolId::LINE);
}

/* method: "tool_spline(AppContext& app)\nSelects the spline tool." */
static void faintapp_tool_spline(AppContext& app){
  app.SelectTool(ToolId::SPLINE);
}

/* method: "tool_rectangle(AppContext& app)\nSelects the rectangle tool." */
static void faintapp_tool_rectangle(AppContext& app){
  app.SelectTool(ToolId::RECTANGLE);
}

/* method: "tool_ellipse(AppContext& app)\nSelects the ellipse tool." */
static void faintapp_tool_ellipse(AppContext& app){
  app.SelectTool(ToolId::ELLIPSE);
}

/* method: "tool_polygon(AppContext& app)\nSelects the polygon tool." */
static void faintapp_tool_polygon(AppContext& app){
  app.SelectTool(ToolId::POLYGON);
}

/* method: "tool_text(AppContext& app)\nSelects the text tool." */
static void faintapp_tool_text(AppContext& app){
  app.SelectTool(ToolId::TEXT);
}

/* method: "tool_fill(AppContext& app)\nSelects the fill tool." */
static void faintapp_tool_fill(AppContext& app){
  app.SelectTool(ToolId::FLOOD_FILL);
}

/* method: "tool_hot_spot(AppContext& app)\nSelects the hot-spot tool." */
static void faintapp_tool_hot_spot(AppContext& app){
  app.SelectTool(ToolId::HOT_SPOT);
}

/* method: "update_settings(settings)\n
Updates the active tool settings with the settings from the passed in
Settings object." */
static void faintapp_update_settings(AppContext& app, const Settings& settings){
  app.UpdateToolSettings(settings);
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

/* method: "__copy__()"
name: "__copy__" */
static void faintapp_copy(AppContext&){
  throw NotImplementedError("FaintApp object can not be copied.");
}

#include "generated/python/method-def/py-app-methoddef.hh"

static void faintapp_init(faintAppObject&) {
  // Prevent instantiation from Python, since the AppContext can't be
  // provided from there.
  throw TypeError(space_sep("FaintApp can not be instantiated.",
    "Use the 'app'-object instead."));
}

static PyObject* faintapp_new(PyTypeObject* type, PyObject*, PyObject*){
  faintAppObject* self = (faintAppObject*)type->tp_alloc(type, 0);
  self->ctx = nullptr;
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

void add_App(AppContext& app, PyObject* module){
  add_type_object(module, FaintAppType, "FaintApp");
  PyModule_AddObject(module, "app",
    create_python_object<faintAppObject>(FaintAppType, app));
}

} // namespace
