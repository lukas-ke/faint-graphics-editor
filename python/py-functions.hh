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

#ifdef FAINT_PY_FUNCTIONS_HH
#error py-functions.hh included twice
#else
#define FAINT_PY_FUNCTIONS_HH
#endif

#include <sstream>
#include <iostream> // for print_to_stdout
#include "python/py-include.hh"
#include "app/get-art-container.hh" // Fixme: \ref(rotate-bad-art-container)
#include "app/canvas.hh"
#include "app/frame.hh"
#include "app/get-app-context.hh"
#include "app/get-python-context.hh"
#include "bitmap/draw.hh"
#include "bitmap/pattern.hh"
#include "geo/pathpt.hh"
#include "geo/offsat.hh"
#include "gui/dialogs.hh"
#include "app/faint-resize-dialog-context.hh" // Fixme
#include "objects/objellipse.hh"
#include "objects/objpath.hh"
#include "objects/objrectangle.hh"
#include "objects/objline.hh"
#include "objects/objraster.hh"
#include "objects/objspline.hh"
#include "objects/objtext.hh"
#include "python/py-bitmap.hh"
#include "python/py-canvas.hh"
#include "python/py-format.hh"
#include "python/py-frame.hh"
#include "python/py-grid.hh"
#include "python/py-object-geo.hh"
#include "python/py-settings.hh"
#include "python/py-util.hh"
#include "python/py-interface.hh"
#include "tools/tool-id.hh"
#include "util/enum-util.hh"
#include "util/image.hh"
#include "util/index-iter.hh"
#include "util/make-vector.hh"
#include "util/optional.hh"
#include "util/settings.hh"
#include "util-wx/clipboard.hh"
#include "util-wx/encode-bitmap.hh"
#include "util-wx/file-path-util.hh"
#include "util-wx/font.hh"
#include "util-wx/key-codes.hh"
#include "generated/python/settings/setting-functions.hh"

namespace faint{

/* function: "add_format(load_callback, save_callback, name, extension)\n
Add a file format for loading and/or saving.\n\n
Either load_callback or save_callback can be None.\n
name is the name of the file format,
extension is the file extension, used to identify the format.\n\n
The load_callback receives a filename and an ImageProps object:\n
my_load_callback(filename, imageProps)\n\nThe save_callback receives a
target filename and a Canvas:\n
my_save_callback(filename, canvas)" */
static void add_format(PyObject*, PyObject* args){
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
  get_app_context().AddFormat(f);
}

/* function: "autosize_raster(raster_object)\n
Trims away same-colored borders around a raster object" */
static void autosize_raster(BoundObject<ObjRaster>& bound){
  python_run_command(bound.Plain(), crop_raster_object_command(bound.obj));
}

/* function: "autosize_text(text_object)\n
Resizes the object's text box to snugly fit the text." */
static void autosize_text(BoundObject<ObjText>& bound){
  python_run_command(bound.Plain(), crop_text_region_command(bound.obj));
}

/* function: "copy_text(s)\n
Copies s to the clipboard." */
static void copy_text(const utf8_string& str){
  Clipboard clipboard;
  if (!clipboard.Good()){
    throw OSError("Failed opening clipboard");
  }
  clipboard.SetText(str);
}

/* function: "dialog_color_balance()\n
Show the color balance dialog." */
static void dialog_color_balance(){
  AppContext& app = get_app_context();
  app.Modal(show_color_balance_dialog);
}

/* function: "dialog_resize()\n
Show the image/selection resize dialog." */
static void dialog_resize(){
  show_resize_dialog(get_app_context());
}

/* function: "dialog_rotate()\n
Show the rotation dialog (for rotating the image or selection)." */
static void dialog_rotate(){
  AppContext& app = get_app_context();
  // Fixme: def(rotate-bad-art-container)Bad get_art_container;
  // get_art_container here is problematic - should be accessed
  // via some context
  app.ModalFull(bind_show_rotate_dialog(get_art_container(),
      app.GetDialogContext()));
}

/* function: "dialog_brightness_contrast()\n
Show the brightness/contrast dialog." */
static void dialog_brightness_contrast(){
  get_app_context().Modal(show_brightness_contrast_dialog);
}

/* function: "dialog_help()\n
Show the help window." */
static void dialog_help(){
  get_app_context().ToggleHelpFrame();
}

/* function: "dialog_open_file()\n
Show the open file dialog." */
static void dialog_open_file(){
  get_app_context().DialogOpenFile();
}

/* function: "dialog_save_file()\n
Show the save file dialog." */
static void dialog_save_file(Canvas* c){
  get_app_context().DialogSaveAs(*c, false);
}

/* function: "dialog_save_copy()\n
Show the save file dialog, without modifying the current file name
when saved." */
static void dialog_save_copy(Canvas* c){
  get_app_context().DialogSaveAs(*c, true);
}

/* function: "dialog_threshold()\n
Show the threshold dialog." */
static void dialog_threshold(){
  AppContext& app = get_app_context();
  // const Settings& s = app.GetToolSettings();
  // Fixme: Pass settings?
  show_threshold_dialog(app.GetDialogContext(), app.GetToolSettings());
}

/* function: "dialog_python_console()\n
Show the Python console." */
static void dialog_python_console(){
  get_app_context().TogglePythonConsole();
}

/* function: "faint_quit()\nExit faint" */
static void faint_quit(){
  get_app_context().Quit();
}

/* function: "get_active_image()\nReturns the active (currently
edited) image." */
static Canvas& get_active_image(){
  return get_app_context().GetActiveCanvas();
}

/* function: "list_images()->(c1,c2, ...),\nReturns a list of all
opened images" */
static std::vector<Canvas*> list_images(){
  AppContext& app = get_app_context();
  return make_vector(up_to(app.GetCanvasCount()),
    [&app](const auto& index){
      return &app.GetCanvas(index);
    });
}

/* function: "get_font()->font\nReturns the active font face name." */
static StringSetting::ValueType get_font(){
  return get_app_context().Get(ts_FontFace);
}

/* function: "get_layer()->i\nReturns the layer index." */
static int get_layer(){
  return static_cast<int>(get_app_context().GetLayerType());
}

/* function: "get_mouse_pos_screen()->x,y\nReturns the mouse pointer
position in screen coordinates." */
static IntPoint get_mouse_pos_screen(){
  return get_app_context().GetMousePos();
}

/* function: "get_settings()\n
Returns a copy of the current tool settings." */
static Settings get_settings(){
  return get_app_context().GetToolSettings();
}

/* function: "set_active_image(image)\n
Activates (selects in a tab) the specified image." */
static void set_active_image(Canvas* canvas){
  get_app_context().SetActiveCanvas(canvas->GetId());
}

/* function: "set_layer(layer)\n
Select layer. 0=Raster, 1=Object" */
static void set_layer(int layer){
  if (!valid_layerstyle(layer)){
    std::stringstream ss;
    ss << "Invalid value for layer.\nAcceptable values are:\n" <<
      to_int(Layer::RASTER) << " - Raster layer\n" <<
      to_int(Layer::OBJECT) << " - Object layer\n";
    throw ValueError(utf8_string(ss.str()));
  }
  get_app_context().SetLayer((to_layerstyle(layer)));
}

/* function: "Faint internal function."
name: "int_incomplete_command" */
static void incomplete_command(){
  get_python_context().Continuation();
}

/* function: "Faint internal function." */
static void int_bind_key(const KeyPress& keyPress){
  // Inform the C++ side that the key is bound
  get_python_context().Bind(keyPress, bind_global(false));
}

/* function: "Faint internal function." */
static void int_bind_key_global(const KeyPress& keyPress){
  // Inform the C++ side that the key is bound globally
  get_python_context().Bind(keyPress, bind_global(true));
}

/* function: "Faint internal function." */
static void int_faint_print(const utf8_string& s){
  get_python_context().IntFaintPrint(s);
}

/* function: "Faint internal function." */
static void int_get_key(){
  get_python_context().GetKey();
}

/* function: "Faint internal function." */
static utf8_string int_get_key_name(int keyCode){
  return KeyPress(Key(keyCode)).Name();
}

/* function: "Faint internal function" */
static void int_unbind_key(const KeyPress& keyPress){
  // Inform the C++ side that the key is unbound
  get_python_context().Unbind(keyPress);
}

/* function: "list_formats()->[file_format, ...]\n
Returns a list of the available file formats as tuples with
(format_name, default_extension)." */
static auto list_formats(){
  return make_vector(get_app_context().GetFormats(),
    [](const Format* f){
      return std::make_pair(f->GetLabel(), f->GetDefaultExtension().Str());
    });
}

/* function: "list_fonts()->[fontname1, ...]\n
Returns a list of the available font names." */
static std::vector<utf8_string> list_fonts(){
  return available_font_facenames();
}

/* function: "cout(s)\nUses std::cout"
name: "cout" */
static void print_to_stdout(const std::string& s){
  std::cout << s << std::endl;
}

/* function: "Faint internal function." */
static void int_ran_command(){
  PythonContext& python(get_python_context());
  python.EvalDone();
  python.NewPrompt();
}

/* function: "swap_colors()\nSwaps the foreground and background colors." */
static void swap_colors(){
  AppContext& app(get_app_context());
  Paint fg(app.Get(ts_Fg));
  Paint bg(app.Get(ts_Bg));
  app.Set(ts_Fg, bg);
  app.Set(ts_Bg, fg);
}

/* function: "blit(src, (x,y), dst)\n
Blits src onto dst with the top left corner of src at (x,y)."
name: "blit" */
static void py_blit_bitmap(const Bitmap& src,
  const IntPoint& topLeft,
  bitmapObject*& dst)
{
  blit(offsat(src, topLeft), onto(*(dst->bmp)));
}

/* function: "tool([i])\n
Selects the tool with the specified id, or returns the current id if
none specified." */
static Optional<int> tool(const Optional<int>& maybeId){
  if (maybeId.NotSet()){
    // Return the currently selected tool identifier
    return option(to_int(get_app_context().GetToolId()));
  }

  int id = maybeId.Get();
  if (!within_enum<ToolId>(id)){
    throw ValueError(space_sep(
      "Invalid tool identifier. Valid tool identifiers are",
      str_int(enum_min_value<ToolId>()), "to", str_int(enum_max_value<ToolId>())));
  }

  get_app_context().SelectTool(to_tool_id(id));

  // Returns nothing when selecting a tool.
  return no_option();
}

/* function: "tool_selection()\nSelects the raster selection tool." */
static void tool_selection(){
  get_app_context().SelectTool(ToolId::SELECTION);
}

/* function: "tool_raster_selection()\nSelects the raster selection tool." */
static void tool_raster_selection(){
  get_app_context().SelectTool(ToolId::SELECTION);
  get_app_context().SetLayer(Layer::RASTER);
}

/* function: "tool_object_selection()\nSelects the object selection tool." */
static void tool_object_selection(){
  get_app_context().SelectTool(ToolId::SELECTION);
  get_app_context().SetLayer(Layer::OBJECT);
}

/* function: "tool_pen()\nSelects the pen tool." */
static void tool_pen(){
  get_app_context().SelectTool(ToolId::PEN);
}

/* function: "tool_brush()\nSelects the brush tool." */
static void tool_brush(){
  get_app_context().SelectTool(ToolId::BRUSH);
}

/* function: "tool_picker()\nSelects the picker tool." */
static void tool_picker(){
  get_app_context().SelectTool(ToolId::PICKER);
}

/* function: "tool_path()\nSelects the path tool." */
static void tool_path(){
  get_app_context().SelectTool(ToolId::PATH);
}

/* function: "tool_level()\nSelects the level tool." */
static void tool_level(){
  get_app_context().SelectTool(ToolId::LEVEL);
}

/* function: "tool_line()\nSelects the line tool." */
static void tool_line(){
  get_app_context().SelectTool(ToolId::LINE);
}

/* function: "tool_spline()\nSelects the spline tool." */
static void tool_spline(){
  get_app_context().SelectTool(ToolId::SPLINE);
}

/* function: "tool_rectangle()\nSelects the rectangle tool." */
static void tool_rectangle(){
  get_app_context().SelectTool(ToolId::RECTANGLE);
}

/* function: "tool_ellipse()\nSelects the ellipse tool." */
static void tool_ellipse(){
  get_app_context().SelectTool(ToolId::ELLIPSE);
}

/* function: "tool_polygon()\nSelects the polygon tool." */
static void tool_polygon(){
  get_app_context().SelectTool(ToolId::POLYGON);
}

/* function: "tool_text()\nSelects the text tool." */
static void tool_text(){
  get_app_context().SelectTool(ToolId::TEXT);
}

/* function: "tool_fill()\nSelects the fill tool." */
static void tool_fill(){
  get_app_context().SelectTool(ToolId::FLOOD_FILL);
}

/* function: "tool_hot_spot()\nSelects the hot-spot tool." */
static void tool_hot_spot(){
  get_app_context().SelectTool(ToolId::HOT_SPOT);
}

// Helper for to_svg_path
static utf8_string points_to_svg_path_string(const std::vector<PathPt>& points){
  std::stringstream ss;
  for (size_t i = 0; i != points.size(); i++){
    const PathPt& pt = points[i];
    if (pt.IsArc()){
      ss << "A" << " " <<
      pt.r.x << " " <<
      pt.r.y << " " <<
      pt.axisRotation.Deg() << " " <<
      pt.largeArcFlag << " " <<
      pt.sweepFlag << " " <<
      pt.p.x << " " <<
      pt.p.y << " ";
    }
    else if (pt.ClosesPath()){
      ss << "z ";
    }
    else if (pt.IsCubicBezier()){
      ss << "C " << pt.c.x << " " << pt.c.y << " " <<
        pt.d.x << " " << pt.d.y << " " <<
        pt.p.x << " " << pt.p.y << " ";
    }
    else if (pt.IsLine()){
      ss << "L " << pt.p.x << " " << pt.p.y << " ";
    }
    else if (pt.IsMove()){
      ss << "M " << pt.p.x << " " << pt.p.y << " ";
    }
    else {
      assert(false); // Invalid PathPt type
    }
  }
  std::string pathStr = ss.str();
  return utf8_string(pathStr.substr(0, pathStr.size() - 1));
}

/* function: "to_svg_path(object)->s\n
Returns an svg-path string describing the object." */
static utf8_string to_svg_path(BoundObject<Object> bound){
  const Image& frame(bound.canvas->GetFrame(bound.frameId));
  auto& ctx(frame.GetExpressionContext());
  return points_to_svg_path_string(bound.obj->GetPath(ctx));
}

/* function: "get_path_points(obj)\n
Returns a list of points describing the object."
name: "get_path_points" */
static std::vector<PathPt> get_path_points(const BoundObject<Object>& bound){
  const Image& frame(bound.canvas->GetFrame(bound.frameId));
  auto& ctx(frame.GetExpressionContext());
  return bound.obj->GetPath(ctx);
}

/* function: "update_settings(settings)\n
Updates the active tool settings with the settings from the passed in
Settings object." */
static void update_settings(const Settings& settings){
  get_app_context().UpdateToolSettings(settings);
}

/* function: "get_config_path()->file_path\n
Returns the path to the user's Python configuration file."
name: "get_config_path" */
extern FilePath get_user_config_file_path();

/* function: "get_config_dir()->folder_path>\n
Returns the path to the directory with the user's Python configuration file."
name: "get_config_dir" */
extern DirPath get_home_dir();

/* function: "one_color_bg(frame)->b\n
True if the frame background consists of a single color." */
static bool one_color_bg(const Image* image){
  return image->GetBackground().Visit(
    [](const Bitmap& bmp){
      return is_blank(bmp);
    },
    [](const ColorSpan&){
      return true;
    });
}

static bool parse_png_bitmap(PyObject* args, Bitmap& out){
  if (PySequence_Length(args) != 1){
    PyErr_SetString(PyExc_TypeError, "A single string argument required.");
    return false;
  }

  PyObject* pngStrPy = PySequence_GetItem(args, 0);
  if (!PyBytes_Check(pngStrPy)){
    py_xdecref(pngStrPy);
    PyErr_SetString(PyExc_TypeError, "Invalid png-string.");
    return false;
  }

  Py_ssize_t len = PyBytes_Size(pngStrPy);
  Bitmap bmp(from_png(PyBytes_AsString(pngStrPy), to_size_t(static_cast<int>(len)))); // Fixme: Check cast/Change type
  py_xdecref(pngStrPy);
  out = bmp;
  return true;
}

/* function: "bitmap_from_png_string(s)->bmp\n
Creates a bitmap from the PNG string." */
static Bitmap bitmap_from_png_string(PyObject*, PyObject* args){
  Bitmap bmp;
  if (!parse_png_bitmap(args, bmp)){
    throw PresetFunctionError();
  }
  return bmp;
}

static bool parse_jpg_bitmap(PyObject* args, Bitmap& out){
  if (PySequence_Length(args) != 1){
    PyErr_SetString(PyExc_TypeError, "A single string argument required.");
    return false;
  }

  PyObject* pngStrPy = PySequence_GetItem(args, 0);
  if (!PyBytes_Check(pngStrPy)){
    py_xdecref(pngStrPy);
    PyErr_SetString(PyExc_TypeError, "Invalid jpg-string.");
    return false;
  }

  Py_ssize_t len = PyBytes_Size(pngStrPy);
  Bitmap bmp(from_jpg(PyBytes_AsString(pngStrPy), to_size_t(static_cast<int>(len)))); // Fixme: Check cast/Change type
  py_xdecref(pngStrPy);
  out = bmp;
  return true;
}

/* function: "bitmap_from_jpg_string(s)->bmp\n
Creates a bitmap from the JPG string." */
static Bitmap bitmap_from_jpg_string(PyObject*, PyObject* args){
  Bitmap bmp;
  if (!parse_jpg_bitmap(args, bmp)){
    throw PresetFunctionError();
  }
  return bmp;
}

/* function: "encode_bitmap_png(bmp)->bytes\n
Returns a Bytes object with with the Bitmap encoded in PNG." */
static std::string encode_bitmap_png(const Bitmap& bmp){
  return to_png_string(bmp);
}

/* function: "get_active_grid()\nReturns a reference which always
targets the grid for the active image." */
static CanvasGrid get_active_grid(){
  return CanvasGrid(nullptr);
}

/* function: "get_object_frame()\n
Returns the frame containing the passed in object." */
static Frame get_object_frame(const BoundObject<Object>& obj){
  return Frame(obj.canvas, obj.canvas->GetFrame(obj.frameId));
}

/* function: "Faint internal." */
static std::string get_pattern_status(){
  std::map<int,int> status = pattern_status();
  std::stringstream ss;
  for (const auto& idToCount : status){
    ss << idToCount.first << "=" << idToCount.second << ",";
  }
  return ss.str();
}

/* extra_include: "generated/python/settings/setting-function-defs.hh" */

} // namespace
