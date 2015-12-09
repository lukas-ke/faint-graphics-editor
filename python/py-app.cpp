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
#include "app/save.hh"
#include "formats/format.hh"
#include "gui/transparency-style.hh"
#include "text/formatting.hh"
#include "tools/tool-id.hh"
#include "util-wx/font.hh"
#include "util-wx/file-path.hh"
#include "util/enum-util.hh"
#include "util/grid.hh"
#include "util/image-info.hh"
#include "util/make-vector.hh"
#include "util/setting-id.hh"
#include "python/py-include.hh"
#include "python/mapped-type.hh"
#include "python/py-add-type-object.hh"
#include "python/py-format.hh"
#include "python/py-func-context.hh"
#include "python/py-ugly-forward.hh"

namespace faint{

class AppContext;

extern PyTypeObject FaintAppType;

struct faintAppObject {
  PyObject_HEAD
  PyFuncContext* ctx;
};

template<>
struct MappedType<PyFuncContext&>{
  using PYTHON_TYPE = faintAppObject;

  static PyFuncContext& GetCppObject(faintAppObject* obj){
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
static void faintapp_add_format(PyFuncContext& ctx, PyObject* args){
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
    FileExtension(extension),
    ctx);
  ctx.app.AddFormat(f);
}

/* method: "close(canvas,force=False)\n
Close the canvas. Asks whether to save changes (message dialog) if the
canvas is modified unless force=True." */
static void faintapp_close(PyFuncContext& ctx, Canvas* canvas,
  const Optional<bool>& force)
{
  ctx.app.Close(*canvas, force.Or(false));
}

/* method: "get_checkered_transparency_indicator()->b\n
True if a checkered pattern is used to indicate transparency." */
static bool faintapp_get_checkered_transparency_indicator(PyFuncContext& ctx){
  return ctx.app.GetTransparencyStyle().IsCheckered();
}

/* method: "list_fonts()->[fontname1, ...]\n
Returns a list of the available font names." */
static std::vector<utf8_string> faintapp_list_fonts(PyFuncContext&){
  return available_font_facenames();
}

/* method: "list_formats()->[file_format, ...]\n
Returns a list of the available file formats as tuples with
(format_name, default_extension)." */
static auto faintapp_list_formats(PyFuncContext& ctx){
  return make_vector(ctx.app.GetFormats(),
    [](const Format* f){
      return std::make_pair(f->GetLabel(), f->GetDefaultExtension().Str());
    });
}

/* method: "new()\n
Create a new Canvas." */
static Bound<Canvas> faintapp_new(PyFuncContext& ctx,
  const Optional<IntSize>& size)
{
  Canvas& c =
    ctx.app.NewDocument(
    ImageInfo(
      size.Or({640, 480}),
      color_white, // Fixme: Allow specifying
      create_bitmap(false)));

  return bind(c, ctx);
}

static FilePath get_openable_path(const utf8_string& s){
  if (!is_file_path(s)){
    throw ValueError(space_sep(quoted(s), "is not a file path"));
  }
  if (!is_absolute_path(s)){
    throw ValueError(space_sep("Path", quoted(s), "not absolute."));
  }
  return FilePath::FromAbsolute(s);
}

/* method: "open_files((file_path1, file_path2,...))\n
Open the specified image files in new tabs." */
static void faintapp_open_files(PyFuncContext& ctx,
  const std::vector<utf8_string>& pathStrings)
{
  FileList paths;
  for (const utf8_string& pathStr : pathStrings){
    paths.push_back(get_openable_path(pathStr));
  }
  if (paths.empty()){
    return;
  }

  ctx.app.Load(paths);
  // Fixme: Return list of Canvas&
}

/* method: "open(file_path)->Image\n
Open the specified image file in a new tab." */
static Optional<Bound<Canvas>> faintapp_open(PyFuncContext& ctx,
  const utf8_string& pathStr)
{
  Canvas* c = ctx.app.Load(get_openable_path(pathStr), change_tab(true));
  if (c == nullptr){
    return no_option();
  }
  return option(bind(*c, ctx));
}

/* method: "quit(force=False)\n
Exit Faint. Asks for confirmation if there are unsaved files unless
force=True." */
static void faintapp_quit(PyFuncContext& ctx, Optional<bool> force){
  ctx.app.Quit(force.Or(false));
}

/* method: "set_transparency_indicator(r,g,b)\n
Sets a color for indicating alpha transparency (instead of a checkered
pattern)" */
static void faintapp_set_transparency_indicator(PyFuncContext& ctx,
  const ColRGB& color)
{
  ctx.app.SetTransparencyStyle(TransparencyStyle(color));
}

/* method: "set_checkered_transparency_indicator()\n
Use a checkered pattern to indicate transparency " */
static void faintapp_set_checkered_transparency_indicator(PyFuncContext& ctx){
  ctx.app.SetTransparencyStyle(TransparencyStyle());
}

// Helper for faintapp_save
static void do_save(PyFuncContext& ctx, Canvas& canvas, const FilePath& filePath){
  SaveResult result = save(canvas,
    ctx.app.GetFormats(),
    filePath);
  if (!result.Successful()){
    throw ValueError(result.ErrorDescription()); // Fixme: Not value error
  }
  canvas.NotifySaved(filePath);
}

/* method: "save(filename)\n
Save the image to the specified file." */
static void faintapp_save(PyFuncContext& ctx,
  Canvas* canvas,
  Optional<FilePath>& maybeFilename)
{
  maybeFilename.Visit(
    [&](const FilePath& path){
      // Use the passed in file-name
      do_save(ctx, *canvas, path);
    },
    [&](){
      // No file name passed in, use the previous canvas file-name, if
      // available.
      canvas->GetFilePath().Visit(
        [&](const FilePath& path){
          do_save(ctx, *canvas, path);
        },
        [](){
          throw ValueError("No filename specified, and no previously used "
            "filename available.");
        });
    });
}

/* method: "save_backup(canvas, filename)\n
Save a copy of the Canvas to the specified file, without changing the
target filename for the Canvas." */
static void faintapp_save_backup(PyFuncContext& ctx,
  Canvas* canvas,
  const FilePath& path)
{
  SaveResult result = save(*canvas, ctx.app.GetFormats(), path);
  if (!result.Successful()){
    throw ValueError(result.ErrorDescription()); // Fixme: Not ValueError
  }
}

/* method: "swap_colors()\n
Swaps the foreground and background colors." */
static void faintapp_swap_colors(PyFuncContext& ctx){
  Paint fg(ctx.app.Get(ts_Fg));
  Paint bg(ctx.app.Get(ts_Bg));
  ctx.app.Set(ts_Fg, bg);
  ctx.app.Set(ts_Bg, fg);
}

/* method: "tool([i])\n
Selects the tool with the specified id, or returns the current id if
none specified." */
static Optional<int> faintapp_tool(PyFuncContext& ctx, const Optional<int>& maybeId){
  if (maybeId.NotSet()){
    // Return the currently selected tool identifier
    return option(to_int(ctx.app.GetToolId()));
  }

  int id = maybeId.Get();
  if (!within_enum<ToolId>(id)){
    throw ValueError(space_sep(
      "Invalid tool identifier. Valid tool identifiers are",
      str_int(enum_min_value<ToolId>()), "to", str_int(enum_max_value<ToolId>())));
  }

  ctx.app.SelectTool(to_tool_id(id));

  // Returns nothing when selecting a tool.
  return no_option();
}

/* method: "tool_selection(PyFuncContext& ctx)\nSelects the raster selection tool." */
static void faintapp_tool_selection(PyFuncContext& ctx){
  ctx.app.SelectTool(ToolId::SELECTION);
}

/* method: "tool_raster_selection(PyFuncContext& ctx)\n
Selects the raster selection tool." */
static void faintapp_tool_raster_selection(PyFuncContext& ctx){
  ctx.app.SelectTool(ToolId::SELECTION);
  ctx.app.SetLayer(Layer::RASTER);
}

/* method: "tool_object_selection(PyFuncContext& ctx)\n
Selects the object selection tool." */
static void faintapp_tool_object_selection(PyFuncContext& ctx){
  ctx.app.SelectTool(ToolId::SELECTION);
  ctx.app.SetLayer(Layer::OBJECT);
}

/* method: "tool_pen(PyFuncContext& ctx)\n
Selects the pen tool." */
static void faintapp_tool_pen(PyFuncContext& ctx){
  ctx.app.SelectTool(ToolId::PEN);
}

/* method: "tool_brush(PyFuncContext& ctx)\n
Selects the brush tool." */
static void faintapp_tool_brush(PyFuncContext& ctx){
  ctx.app.SelectTool(ToolId::BRUSH);
}

/* method: "tool_picker(PyFuncContext& ctx)\n
Selects the picker tool." */
static void faintapp_tool_picker(PyFuncContext& ctx){
  ctx.app.SelectTool(ToolId::PICKER);
}

/* method: "tool_path(PyFuncContext& ctx)\n
Selects the path tool." */
static void faintapp_tool_path(PyFuncContext& ctx){
  ctx.app.SelectTool(ToolId::PATH);
}

/* method: "tool_level(PyFuncContext& ctx)\n
Selects the level tool." */
static void faintapp_tool_level(PyFuncContext& ctx){
  ctx.app.SelectTool(ToolId::LEVEL);
}

/* method: "tool_line(PyFuncContext& ctx)\n
Selects the line tool." */
static void faintapp_tool_line(PyFuncContext& ctx){
  ctx.app.SelectTool(ToolId::LINE);
}

/* method: "tool_spline(PyFuncContext& ctx)\n
Selects the spline tool." */
static void faintapp_tool_spline(PyFuncContext& ctx){
  ctx.app.SelectTool(ToolId::SPLINE);
}

/* method: "tool_rectangle(PyFuncContext& ctx)\n
Selects the rectangle tool." */
static void faintapp_tool_rectangle(PyFuncContext& ctx){
  ctx.app.SelectTool(ToolId::RECTANGLE);
}

/* method: "tool_ellipse(PyFuncContext& ctx)\n
Selects the ellipse tool." */
static void faintapp_tool_ellipse(PyFuncContext& ctx){
  ctx.app.SelectTool(ToolId::ELLIPSE);
}

/* method: "tool_polygon(PyFuncContext& ctx)\n
Selects the polygon tool." */
static void faintapp_tool_polygon(PyFuncContext& ctx){
  ctx.app.SelectTool(ToolId::POLYGON);
}

/* method: "tool_text(PyFuncContext& ctx)\n
Selects the text tool." */
static void faintapp_tool_text(PyFuncContext& ctx){
  ctx.app.SelectTool(ToolId::TEXT);
}

/* method: "tool_fill(PyFuncContext& ctx)\n
Selects the fill tool." */
static void faintapp_tool_fill(PyFuncContext& ctx){
  ctx.app.SelectTool(ToolId::FLOOD_FILL);
}

/* method: "tool_hot_spot(PyFuncContext& ctx)\n
Selects the hot-spot tool." */
static void faintapp_tool_hot_spot(PyFuncContext& ctx){
  ctx.app.SelectTool(ToolId::HOT_SPOT);
}

/* method: "update_settings(settings)\n
Updates the active tool settings with the settings from the passed in
Settings object." */
static void faintapp_update_settings(PyFuncContext& ctx, const Settings& settings){
  ctx.app.UpdateToolSettings(settings);
}

/* property: "gridcolor\n
The default color of grids used for new documents." */
struct faintapp_gridcolor{
  static Color Get(PyFuncContext& ctx){
    return ctx.app.GetDefaultGrid().GetColor();
  }

  static void Set(PyFuncContext& ctx, const Color& color){
    Grid grid(ctx.app.GetDefaultGrid());
    grid.SetColor(color);
    ctx.app.SetDefaultGrid(grid);
  }
};

/* property: "griddashed\n
Whether grids used for new documents are dashed or solid." */
struct faintapp_griddashed{
  static bool Get(PyFuncContext& ctx){
    return ctx.app.GetDefaultGrid().Dashed();
  }

  static void Set(PyFuncContext& ctx, bool dashed){
    Grid grid(ctx.app.GetDefaultGrid());
    grid.SetDashed(dashed);
    ctx.app.SetDefaultGrid(grid);
  }
};

/* method: "__copy__() Not implemented."
name: "__copy__" */
static void faintapp_special_copy(PyFuncContext&){
  throw NotImplementedError("FaintApp object can not be copied.");
}

#include "generated/python/method-def/py-app-method-def.hh"

static void faintapp_special_init(faintAppObject&) {
  // Prevent instantiation from Python, since the AppContext can't be
  // provided from there.
  throw TypeError(space_sep("FaintApp can not be instantiated.",
    "Use the 'app'-object instead."));
}

static PyObject* faintapp_special_new(PyTypeObject* type, PyObject*, PyObject*){
  faintAppObject* self = (faintAppObject*)type->tp_alloc(type, 0);
  self->ctx = nullptr;
  return (PyObject*)self;
}

static utf8_string faintapp_special_repr(PyFuncContext&){
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
    REPR_FORWARDER(faintapp_special_repr), // tp_repr
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
    INIT_FORWARDER(faintapp_special_init), // tp_init
    nullptr, // tp_alloc
    faintapp_special_new, // tp_new
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

void add_App(PyFuncContext& ctx, PyObject* module){
  add_type_object(module, FaintAppType, "FaintApp");
  PyModule_AddObject(module, "app",
    create_python_object<faintAppObject>(FaintAppType, ctx));
}

} // namespace
