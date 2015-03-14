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

#include "app/canvas.hh"
#include "app/frame.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/gradient.hh"
#include "bitmap/paint.hh"
#include "bitmap/pattern.hh"
#include "geo/arc.hh"
#include "geo/calibration.hh"
#include "geo/int-rect.hh"
#include "geo/line.hh"
#include "geo/pathpt.hh"
#include "geo/rect.hh"
#include "python/py-include.hh"
#include "python/py-function-error.hh"
#include "python/py-canvas.hh"
#include "python/py-grid.hh"
#include "python/py-linear-gradient.hh"
#include "python/py-radial-gradient.hh"
#include "python/py-util.hh"
#include "python/py-parse.hh"
#include "python/py-pattern.hh"
#include "python/py-settings.hh"
#include "python/py-something.hh"
#include "python/py-tri.hh"
#include "python/py-frame.hh"
#include "text/text-line.hh"
#include "util/color-span.hh"
#include "util/grid.hh"
#include "util/key-press.hh"
#include "util/settings.hh"
#include "util/plain-type.hh"
#include "util-wx/file-path.hh"

namespace faint{
const TypeName arg_traits<Object>::name("Object");
const TypeName arg_traits<BoundObject<Object>>::name(arg_traits<Object>::name);

const TypeName arg_traits<bitmapObject>::name("Bitmap");
const TypeName arg_traits<Canvas>::name("Canvas");
const TypeName arg_traits<coord>::name("coordinate");
const TypeName arg_traits<BoundObject<ObjRaster>>::name("Raster object");
const TypeName arg_traits<BoundObject<ObjText>>::name("Text object");
const TypeName arg_traits<Grid>::name("Grid");
const TypeName arg_traits<Image>::name("Image");
const TypeName arg_traits<Index>::name("Index");
const TypeName arg_traits<IntLineSegment>::name("Line");
const TypeName arg_traits<LineSegment>::name("Line");
const TypeName arg_traits<Paint>::name("Paint");
const TypeName arg_traits<Settings>::name("Settings");
const TypeName arg_traits<Tri>::name("Tri");
const TypeName arg_traits<utf8_string>::name("str");

template<typename T>
TypeName type_name(const T&){
  return arg_traits<T>::name;
}

template<typename T>
TypeName type_name(const T* const){
  return arg_traits<T>::name;
}

template<typename T>
TypeName type_name(T*&){
  return arg_traits<T>::name;
}

static PyObject* build_frame(Canvas* canvas, const FrameId& frameId){
  frameObject* py_frame = (frameObject*)(FrameType.tp_alloc(&FrameType, 0));
  py_frame->canvas = canvas;
  py_frame->canvasId = canvas->GetId();
  py_frame->frameId = frameId;
  return (PyObject*)py_frame;
}

static PyObject* build_gradient(const Gradient& gradient){
  if (gradient.IsLinear()){
    linearGradientObject* py_gradient =
      (linearGradientObject*)LinearGradientType.tp_alloc(&LinearGradientType, 0);
    py_gradient->gradient = new LinearGradient(gradient.GetLinear());
    return (PyObject*)py_gradient;
  }
  else if (gradient.IsRadial()){
    radialGradientObject* py_gradient =
      (radialGradientObject*)RadialGradientType.tp_alloc(&RadialGradientType, 0);
    py_gradient->gradient = new RadialGradient(gradient.GetRadial());
    return (PyObject*)py_gradient;
  }
  else {
    assert(false);
    return Py_BuildValue("");
  }
}

static PyObject* build_pattern(const Pattern& pattern){
  patternObject* py_pattern =
    (patternObject*)PatternType.tp_alloc(&PatternType, 0);
  py_pattern->pattern = new Pattern(pattern);
  return (PyObject*)py_pattern;
}

static PyObject* build_color_tuple(const Color& color){
  return Py_BuildValue("iiii", color.r, color.g, color.b, color.a);
}

static PyObject* build_paint(const Paint& paint){
  return visit(paint, build_color_tuple, build_pattern, build_gradient);
}

bool parse_item(PyObject*& item, PyObject* args, int&, int, bool){
  item = args;
  return true;
}

bool parse_int(PyObject* args, int n, int* value){
  PyObject* obj = PySequence_GetItem(args, n);
  long temp = PyLong_AsLong(obj);
  py_xdecref(obj);
  if (temp == -1 && PyErr_Occurred()){
    return false;
  }
  *value = static_cast<int>(temp); // Fixme: Check cast or change type
  return true;
}

bool parse_bool(PyObject* args, int n, bool* value){
  PyObject* obj = PySequence_GetItem(args, n);
  *value = (1 == PyObject_IsTrue(obj));
  py_xdecref(obj);
  return true;
}

bool parse_Index(PyObject* args, int n, Index* value){
  PyObject* obj = PySequence_GetItem(args, n);
  long temp = PyLong_AsLong(obj);
  py_xdecref(obj);
  if (temp == -1 && PyErr_Occurred()){
    return false;
  }
  if (temp < 0){
    throw ValueError("Negative index specified.", n);
  }
  *value = Index(static_cast<int>(temp)); // Fixme: Use conversion function
  return true;
}

bool parse_coord(PyObject* args, int n, coord* value){
  if (PySequence_Check(args)){
    PyObject* obj = PySequence_GetItem(args, n);
    coord temp = PyFloat_AsDouble(obj);
    py_xdecref(obj);
    if (PyErr_Occurred()){
      return false;
    }
    *value = temp;
    return true;
  }
  else if (PyNumber_Check(args)){
    // Fixme: This is a weird special case to support setters. Instead
    // figure out when an arg-list is being parsed, vs. a type.
    *value = PyFloat_AsDouble(args);
    if (*value == -1.0){
      if (PyErr_Occurred()){
        return false;
      }
    }
    return true;
  }
  else{
    throw TypeError(arg_traits<coord>::name, n);
  }
}

bool parse_bytes(PyObject* args, int, std::string* value){
  char* str = PyBytes_AsString(args);
  if (str == nullptr){
    // PyBytes_AsString will have raised TypeError
    return false;
  }
  *value = std::string(str);
  return true;
}

bool parse_flat(bitmapObject*& bmp, PyObject* args, int& n, int len){
  throw_insufficient_args_if(len - n < 1, "bitmap");

  scoped_ref ref(PySequence_GetItem(args, n));
  if (!PyObject_IsInstance(ref.get(), (PyObject*)&BitmapType)){
    throw TypeError(type_name(bmp), n);
  }

  bmp = (bitmapObject*)(ref.get()); // Fixme: Looks dangerous
  return true;
}

bool parse_flat(BoundObject<Object>& obj, PyObject* args, int& n, int len){
  throw_insufficient_args_if(len - n < 1, "Object");

  if (!PySequence_Check(args)){
    if (!PyObject_IsInstance(args, (PyObject*)&SmthType)){
      throw TypeError(type_name(obj), n);
    }
    smthObject* smth = (smthObject*)(args);
    obj = BoundObject<Object>(smth->canvas, smth->obj, smth->frameId);
    return true;
  }

  scoped_ref ref(PySequence_GetItem(args, n));
  if (!PyObject_IsInstance(ref.get(), (PyObject*)&SmthType)){
    throw TypeError(arg_traits<Object>::name, n);
  }

  smthObject* smth = (smthObject*)(ref.get());
  obj = BoundObject<Object>(smth->canvas, smth->obj, smth->frameId);
  return true;
}

bool parse_flat(Calibration& calibration, PyObject* args, int& n, int len){
  throw_insufficient_args_if(len - n < 3, "Calibration");
  coord x0, y0, x1, y1, lineLength;
  PyObject* unitStr;
  if (!PyArg_ParseTuple(args, "(dddd)dO",
      &x0, &y0, &x1, &y1,
      &lineLength,
      &unitStr)){
    throw PresetFunctionError();
  }

  PyObject* utf8 = PyUnicode_AsUTF8String(unitStr);
  if (utf8 == nullptr){
    throw PresetFunctionError();
  }
  char* bytes = PyBytes_AsString(utf8);
  utf8_string unit(bytes);
  calibration = Calibration(LineSegment(Point(x0,y0), Point(x1,y1)), lineLength,
    unit);
  return true;

}

bool parse_flat(Canvas*& canvas, PyObject* args, int& n, int len){
  throw_insufficient_args_if(len - n < 1, "Canvas");

  scoped_ref ref(PySequence_GetItem(args, n));
  if (!PyObject_IsInstance(ref.get(), (PyObject*)&CanvasType)){
    throw TypeError(arg_traits<Canvas>::name, n);
  }

  canvasObject* pyCanvas = (canvasObject*)(ref.get());
  if (!canvas_ok(pyCanvas->id)){
    throw ValueError("Operation on closed canvas.");
  }
  n += 1;
  canvas = pyCanvas->canvas;
  return true;
}

bool parse_flat(Grid& grid, PyObject* args, int& n, int len){
  throw_insufficient_args_if(len - n < 1, "grid");

  if (!PyObject_IsInstance(args, (PyObject*)&GridType)){
    throw TypeError(type_name(grid), n);
  }
  Optional<Grid> maybeGrid = get_grid((gridObject*)args);
  if (!maybeGrid.IsSet()){
    return false;
  }
  grid = maybeGrid.Get();
  n += 1;
  return true;
}

bool parse_flat(DefaultConstructible<Angle>& angle,
  PyObject* args,
  int& n, int len)
{
  throw_insufficient_args_if(len - n < 1, "Angle");
  coord radians;
  if (parse_coord(args, n, &radians)){
    n += 1;
    angle.Set(Angle::Rad(radians));
    return true;
  }
  return false;
}

bool parse_flat(DefaultConstructible<Delay>& delay,
  PyObject* args,
  int& n, int len)
{
  throw_insufficient_args_if(len - n < 1, "Delay");
  Delay::value_type v;
  if (parse_int(args, n, &v)){
    n += 1;
    delay.Set(Delay(v));
    return true;
  }
  return false;
}

bool parse_flat(DefaultConstructible<FilePath>& p,
  PyObject* args,
  int& n,
  int len)
{
  throw_insufficient_args_if(len - n < 1, "File path");

  // Fixme: Check error handling behavior
  // Fixme: decref
  PyObject* utf8 = PyUnicode_AsUTF8String(args);
  if (utf8 == nullptr){
    throw PresetFunctionError();
  }
  char* bytes = PyBytes_AsString(utf8);
  utf8_string str(bytes);

  if (!is_absolute_path(str)){
    throw ValueError(space_sep(quoted(str), "is not absolute."));
  }
  if(!is_file_path(str)){
    throw ValueError(space_sep(quoted(str), "is not a valid file name."));
  }

  p.Set(FilePath::FromAbsolute(str));
  return true;
}

bool parse_flat(const Image*& image, PyObject* args, int& n, int len){
  throw_insufficient_args_if(len - n < 1, "Canvas");

  scoped_ref ref(PySequence_GetItem(args, n));
  if (!PyObject_IsInstance(ref.get(), (PyObject*)&FrameType)){
    throw TypeError(type_name(image), n);
  }

  frameObject* pyFrame = (frameObject*)(ref.get());
  if (expired(pyFrame)){
    throw ValueError("Operation on closed Frame.");
  }
  image = &(pyFrame->canvas->GetFrame(pyFrame->frameId));
  return true;
}

bool parse_flat(bool& value, PyObject* args, int& n, int len){
  throw_insufficient_args_if(len - n < 1, "boolean");

  if (PySequence_Check(args)){
    if (!parse_bool(args, n, &value)){
      return false;
    }
    n += 1;
    return true;
  }
  else{
    value = (1 == PyObject_IsTrue(args));
    n += 1;
    return true;
  }
}

bool parse_flat(int& value, PyObject* args, int& n, int len){
  throw_insufficient_args_if(len - n < 1, "integer");

  if (PySequence_Check(args)){
    if (!parse_int(args, n, &value)){
      return false;
    }
    n += 1;
    return true;
  }

  long temp = PyLong_AsLong(args);
  if (temp == -1 && PyErr_Occurred()){
    return false;
  }
  value = static_cast<int>(temp); // Fixme: Check cast or change type
  n += 1;
  return true;
}

bool parse_flat(Index& value, PyObject* args, int& n, int len){
  throw_insufficient_args_if(len - n < 1, "index");

  if (PySequence_Check(args)){
    if (!parse_Index(args, n, &value)){
      return false;
    }
    n += 1;
    return true;
  }

  long temp = PyLong_AsLong(args);
  if (temp == -1 && PyErr_Occurred()){
    return false;
  }
  if (temp < 0){
    throw ValueError("Negative index specified.", n);
  }
  value = Index(static_cast<int>(temp)); // Fixme: Use conversion function
  n += 1;
  return true;
}

bool parse_flat(coord& value, PyObject* args, int& n, int len){
  throw_insufficient_args_if(len - n < 1, "float");

  if (parse_coord(args, n, &value)){
    n += 1;
    return true;
  }
  return false;
}

bool parse_flat(IntSize& size, PyObject* args, int& n, int len){
  throw_insufficient_args_if(len - n < 2, "size");

  int w, h;
  if (!parse_int(args, n, &w) ||!parse_int(args, n+1, &h)){
    return false;
  }
  if (w <= 0 || h <= 0){
    throw ValueError("Size must be positive", n);
  }
  size.w = w;
  size.h = h;
  return true;
}

bool parse_flat(IntRect& r, PyObject* args, int& n, int len){
  throw_insufficient_args_if(len - n < 4, "IntRect");

  int x, y, w, h;
  if (!parse_int(args, n, &x) ||
    !parse_int(args, n + 1, &y) ||
    !parse_int(args, n + 2, &w) ||
    !parse_int(args, n + 3, &h)){
    // Note: PyErr already set by parse_int
    return false;
  }

  if (w < 0){
    throw ValueError("Negative rectangle width specified.");
  }
  if (h < 0){
    throw ValueError("Negative rectangle height specified.");
  }

  n += 4;
  r.x = x;
  r.y = y;
  r.w = w;
  r.h = h;
  return true;
}

bool parse_flat(Tri& tri, PyObject* args, int& n, int len){
  throw_insufficient_args_if(len - n < 1, "Tri");
  if (PyObject_IsInstance(args, (PyObject*)&TriType)){
    tri = ((triObject*)args)->tri;
    return true;
  }

  scoped_ref ref(PySequence_GetItem(args, n));
  if (!PyObject_IsInstance(ref.get(), (PyObject*)&TriType)){
    throw TypeError(type_name(tri), n);
  }
  if (!valid(((triObject*)(ref.get()))->tri)){
    throw ValueError("Invalid Tri.");
  }
  tri = ((triObject*)(ref.get()))->tri;
  n += 1;
  return true;
}

bool parse_flat(Radii& radii, PyObject* args, int& n, int len){
  throw_insufficient_args_if(len - n < 2, "Radii");

  coord rx, ry;
  if (!parse_coord(args, n, &rx) || !parse_coord(args, n+1, &ry)){
    return false;
  }

  radii.x = rx;
  radii.y = ry;
  return true;
}

bool parse_flat(Color& color, PyObject* args, int& n, int len){
  const int items = len - n;
  throw_insufficient_args_if(items < 3, "color");

  int readItems = 0;

  int r, g, b;
  int a = 255;
  if (!parse_int(args, n, &r) ||
    !parse_int(args, n + 1, &g) ||
    !parse_int(args, n + 2, &b)){
    return false;
  }
  readItems += 3;

  if (items >= 4){
    if (!parse_int(args, n + 3, &a)){
      return false;
    }
    readItems += 1;
  }
  if (!valid_color(r,g,b,a)){
    throw ValueError("Invalid color", n);
  }

  n += readItems;
  color = color_from_ints(r,g,b,a);
  return true;
}

bool parse_flat(ColorSpan& colorSpan, PyObject* args, int& n, int len){
  const int numItems = len - n;
  throw_insufficient_args_if(numItems < 2, "ColorSpan");

  scoped_ref maybeColor(PySequence_GetItem(args, n));
  scoped_ref maybeSize(PySequence_GetItem(args, n + 1));

  Color color;
  int n2 = 0;
  if (!parse_flat(color, maybeColor.get(), n2,
    static_cast<int>(PySequence_Length(maybeColor.get())))) // Fixme: Check cast/Change type
  {
    throw TypeError("First item of ColorSpan must be a color.");
  }

  IntSize size;
  int n3 = 0;
  if (!parse_flat(size, maybeSize.get(), n3,
    static_cast<int>(PySequence_Length(maybeSize.get())))) // Fixme: Check cast/Change type
  {
    throw TypeError("Second item of ColorSpan must be a size.");
  }

  colorSpan = ColorSpan(color, size);
  n += 2;
  return true;
}

bool parse_flat(Rect& r, PyObject* args, int& n, int len){
  throw_insufficient_args_if(len - n < 4, "Rect");

  coord x, y, w, h;
  if (!parse_coord(args, n, &x) ||
    !parse_coord(args, n + 1, &y) ||
    !parse_coord(args, n + 2, &w) ||
    !parse_coord(args, n + 3, &h)){
    // Note: PyErr already set by parse_coord
    return false;
  }

  if (w < 0){
    throw ValueError("Negative rectangle width specified.");
  }
  if (h < 0){
    throw ValueError("Negative rectangle height specified.");
  }

  n += 4;
  r.x = x;
  r.y = y;
  r.w = w;
  r.h = h;
  return true;
}

static Pattern* as_Pattern(PyObject* obj){
  if (!PyObject_IsInstance(obj, (PyObject*)&PatternType)){
    PyErr_SetString(PyExc_TypeError, "The argument must be a Pattern object");
    return nullptr;
  }
  patternObject* pyPattern = (patternObject*)(obj);
  return pyPattern->pattern;
}

static Optional<Gradient> as_Gradient(PyObject* obj){
  if (PyObject_IsInstance(obj, (PyObject*)&LinearGradientType)){
    linearGradientObject* pyGradient = (linearGradientObject*)(obj);
    return option(Gradient(*pyGradient->gradient));
  }
  else if (PyObject_IsInstance(obj, (PyObject*)&RadialGradientType)){
    radialGradientObject* pyGradient = (radialGradientObject*)(obj);
    return option(Gradient(*pyGradient->gradient));
  }

  PyErr_SetString(PyExc_TypeError,
    "The argument must be a LinearGradient or a RadialGradient object");
  return no_option();
}

static bool parse_non_sequence(Paint& paint, PyObject* arg){
  Optional<Gradient> gradient = as_Gradient(arg);
  if (gradient.IsSet()){
    paint = Paint(gradient.Get());
    return true;
  }

  // Try parsing as pattern instead
  PyErr_Clear();
  Pattern* pattern = as_Pattern(arg);
  if (pattern != nullptr){
    paint = Paint(*pattern);
    return true;
  }
  return false;
}

bool parse_flat(Paint& paint, PyObject* args, int& n, int len){
  if (!PySequence_Check(args)){
    if (parse_non_sequence(paint, args)){
      return true;
    }
    else{
      throw TypeError("Expected a color, pattern or gradient.");
    }
  }
  else if (len - n == 1){
    scoped_ref ref(PySequence_GetItem(args, n));
    Optional<Gradient> gradient = as_Gradient(ref.get());
    if (gradient.IsSet()){
      paint = Paint(gradient.Get());
      n += 1;
      return true;
    }

    PyErr_Clear();
    Pattern* pattern = as_Pattern(ref.get());
    if (pattern != nullptr){
      paint = Paint(*pattern);
      n += 1;
      return true;
    }
    throw TypeError(type_name(paint), n);
  }
  else {
    Color c;
    if (parse_flat(c, args, n, len)){ // Fixme: For some reason wrong n if error
      paint = Paint(c);
      return true;
    }
    return false;
  }
}

bool parse_flat(IntLineSegment& line, PyObject* args, int& n, int len){
  throw_insufficient_args_if(len - n < 4, "line segment");

  int x0;
  int y0;
  int x1;
  int y1;
  if (!parse_int(args, n, &x0) ||
    !parse_int(args, n + 1, &y0) ||
    !parse_int(args, n + 2, &x1) ||
    !parse_int(args, n + 3, &y1)){

    throw TypeError(type_name(line), "requires four coordinates", n);
  }

  n += 4;
  line.p0.x = x0;
  line.p0.y = y0;
  line.p1.x = x1;
  line.p1.y = y1;
  return true;
}

bool parse_flat(IntPoint& pt, PyObject* args, int& n, int len){
  throw_insufficient_args_if(len - n < 2, "IntPoint");

  int x, y;
  if (!parse_int(args, n, &x) ||
    !parse_int(args,n+1, &y)){
    return false;
  }
  pt.x = x;
  pt.y = y;
  return true;
}

bool parse_flat(ColorStop& stop, PyObject* args, int& n, int len){
  throw_insufficient_args_if(len - n < 2, "color stop");

  double offset;
  int r,g,b;
  int a = 255;

  if (!PyArg_ParseTuple(args + n, "d(iii)", &offset, &r, &g, &b)){
    PyErr_Clear();
    if (!PyArg_ParseTuple(args + n, "d(iiii)", &offset, &r, &g, &b, &a)){
      PyErr_Clear();
      throw ValueError("Invalid color stop", n);
    }
  }

  if (offset < 0.0 || 1.0 < offset){
    throw ValueError("Offset must be in range 0.0-1.0", n);
  }
  if (invalid_color(r,g,b,a)){
    throw ValueError("Invalid color specified", n);
  }
  stop = ColorStop(color_from_ints(r,g,b,a), offset);
  return true;
}

bool parse_flat(LineSegment& line, PyObject* args, int& n, int len){
  throw_insufficient_args_if(len - n < 4, "line segment");

  coord x0;
  coord y0;
  coord x1;
  coord y1;
  if (!parse_coord(args, n, &x0) ||
    !parse_coord(args, n + 1, &y0) ||
    !parse_coord(args, n + 2, &x1) ||
    !parse_coord(args, n + 3, &y1)){

    throw TypeError(type_name(line), "requires four coordinates", n);
  }

  n += 4;
  line.p0.x = x0;
  line.p0.y = y0;
  line.p1.x = x1;
  line.p1.y = y1;
  return true;
}

bool parse_flat(Point& pt, PyObject* args, int& n, int len){
  throw_insufficient_args_if(len - n < 2, "IntPoint");

  coord x, y;
  if (!parse_coord(args, n, &x) ||
    !parse_coord(args,n+1, &y)){
    return false;
  }
  pt.x = x;
  pt.y = y;
  return true;
}

bool parse_flat(ColRGB& color, PyObject* args, int& n, int len){
  throw_insufficient_args_if(len - n < 3, "RGB-color");

  int r, g, b;
  if (!parse_int(args, n, &r) ||
    !parse_int(args, n + 1, &g) ||
    !parse_int(args, n + 2, &b)){

    return false;
  }

  if (!valid_color(r,g,b)){
    throw ValueError("Color component outside valid range", n);
  }

  color = rgb_from_ints(r,g,b);
  return true;
}

bool parse_flat(std::string& value, PyObject* args, int& n, int len){
  throw_insufficient_args_if(len - n < 1, "byte string");

  if (parse_bytes(args, n, &value)){
    n += 1;
    return true;
  }
  return false;
}

bool parse_flat(utf8_string& value, PyObject* args, int& n, int /*len*/){
  if (n != 0){
    throw ValueError("Flat string parse not starting at 0?");
  }

  return parse_py_unicode(args).Visit(
    [&value, &n](const utf8_string& s){
      value = s;
      n += 1;
      return true;
    },
    [&value, &n]() -> bool{
      PyErr_Clear(); // Fixme: Remove error-setting in parse_py_unicode
      throw TypeError(type_name(value), n);
    });
}

bool parse_flat(Settings& s, PyObject* args, int& n, int len){
  throw_insufficient_args_if(len - n < 1, "Settings");

  scoped_ref ref(PySequence_GetItem(args, n));
  if (!PyObject_IsInstance(ref.get(), (PyObject*)&SettingsType)){
    throw TypeError(type_name(s), n);
  }

  s = *(((settingsObject*)(ref.get()))->settings);
  n += 1;
  return true;
}

static Bitmap* as_Bitmap(PyObject* obj, int n){
  if (!PyObject_IsInstance(obj, (PyObject*)&BitmapType)){

    PyErr_SetString(PyExc_ValueError, space_sep("Argument", str_int(n + 1), "must be a Bitmap").c_str());
    return nullptr;
  }
  bitmapObject* py_bitmap = (bitmapObject*)obj;
  return py_bitmap->bmp;
}

bool parse_flat(Bitmap& bmp, PyObject* args, int& n, int len){
  throw_insufficient_args_if(len - n < 1, "Bitmap");

  scoped_ref ref(PySequence_GetItem(args, n));
  Bitmap* tempBmp = as_Bitmap(ref.get(), n);
  if (tempBmp == nullptr){
    return false;
  }
  n += 1;

  bmp = *tempBmp; // Fixme: Unnecessary copy
  return true;
}

bool parse_flat(AngleSpan& span, PyObject* args, int& n, int len){
  throw_insufficient_args_if(len - n < 2, "angle span");

  coord a1;
  coord a2;

  if (!parse_coord(args, n, &a1) || !parse_coord(args, n + 1, &a2)){
    return false;
  }
  n += 2;
  span.start = Angle::Rad(a1);
  span.stop = Angle::Rad(a2);
  return true;
}

bool parse_flat(KeyPress& value, PyObject* args, int& n, int len){
  throw_insufficient_args_if(len - n < 2, "KeyPress");

  int keyCode, modifier;
  if (!parse_int(args, n, &keyCode) || !parse_int(args, n + 1, &modifier)){
    return false;
  }

  // Fixme: Add sanity check for code and modifiers
  n += 2;
  value = KeyPress(Mod::Create(modifier), Key(keyCode));
  return true;
}

PyObject* build_result(const Angle& angle){
  return build_result(angle.Rad());
}

PyObject* build_result(const Delay& delay){
  return build_result(delay.Get());
}

PyObject* build_result(bool value){
  if (value){
    Py_RETURN_TRUE;
  }
  else{
    Py_RETURN_FALSE;
  }
}

PyObject* build_result(const Bitmap& bmp){
  assert(bitmap_ok(bmp));
  Bitmap* newBitmap = nullptr;
  try{
    newBitmap = new Bitmap(bmp);
  }
  catch(const std::bad_alloc&){
    PyErr_SetString(PyExc_MemoryError,
      "Insufficient memory for allocating Bitmap");
    return nullptr;
  }
  bitmapObject* py_bitmap = (bitmapObject*)BitmapType.tp_alloc(&BitmapType, 0);
  if (py_bitmap == nullptr){
    return nullptr;
  }
  py_bitmap->bmp = newBitmap;
  return (PyObject*)py_bitmap;
}

PyObject* build_result(const BoundObject<Object>& obj){
  return pythoned(obj.obj, obj.canvas, obj.frameId);
}

PyObject* build_result(Canvas& canvas){
  return pythoned(canvas);
}

PyObject* build_result(const Calibration& c){
  PyObject* list(PyList_New(3));
  PyList_SetItem(list, 0, build_result(c.pixelLine));
  PyList_SetItem(list, 1, build_result(c.length));
  PyList_SetItem(list, 2, build_result(c.unit));
  return list;
}

PyObject* build_result(Canvas* canvas){
  return pythoned(*canvas);
}

PyObject* build_result(const CanvasGrid& grid){
  return py_grid_canvas(grid.canvas);
}

PyObject* build_result(const Color& color){
  return build_color_tuple(color);
}

PyObject* build_result(const ColorStop& stop){
  Color c(stop.GetColor());
  double offset(stop.GetOffset());
  return Py_BuildValue("d(iiii)", offset, c.r, c.g, c.b, c.a);
}

PyObject* build_result(coord value){
  return Py_BuildValue("d", value);
}

PyObject* build_result(const DirPath& dirPath){
  return build_unicode(dirPath.Str());
}

PyObject* build_result(const FilePath& filePath){
  return build_unicode(filePath.Str());
}

PyObject* build_result(const Frame& frame){
  return build_frame(frame.canvas, frame.frameId);
}

PyObject* build_result(const Index& index){
  return build_result(index.Get());
}

PyObject* build_result(int value){
  return Py_BuildValue("i", value);
}

PyObject* build_result(const IntPoint& pos){
  return Py_BuildValue("ii", pos.x, pos.y);
}

PyObject* build_result(const IntRect& rect){
  return Py_BuildValue("iiii",rect.x, rect.y, rect.w, rect.h);
}

PyObject* build_result(const IntSize& size){
  return Py_BuildValue("ii", size.w, size.h);
}

PyObject* build_result(const LineSegment& l){
  return Py_BuildValue("dddd", l.p0.x, l.p0.y, l.p1.x, l.p1.y);
}

PyObject* build_result(const TextLine& line){
  int hardBreak = line.hardBreak ? 1 : 0;
  return Py_BuildValue("(iOd)", hardBreak, build_result(line.text),
    line.width);
}

PyObject* build_result(const Paint& paint){
  return build_paint(paint);
}

PyObject* build_result(const PathPt& pt){
    if (pt.IsArc()){
      PyObject* arcTo = PyList_New(8);
      PyList_SetItem(arcTo, 0, PyUnicode_FromString("A"));
      PyList_SetItem(arcTo, 1, PyFloat_FromDouble(pt.p.x));
      PyList_SetItem(arcTo, 2, PyFloat_FromDouble(pt.p.y));
      PyList_SetItem(arcTo, 3, PyFloat_FromDouble(pt.r.x));
      PyList_SetItem(arcTo, 4, PyFloat_FromDouble(pt.r.y));
      PyList_SetItem(arcTo, 5, PyFloat_FromDouble(pt.axisRotation.Deg()));
      PyList_SetItem(arcTo, 6, PyLong_FromLong(pt.largeArcFlag));
      PyList_SetItem(arcTo, 7, PyLong_FromLong(pt.sweepFlag));
      return arcTo;
    }
    else if (pt.ClosesPath()){
      PyObject* close = PyList_New(1);
      PyList_SetItem(close, 0, PyUnicode_FromString("Z"));
      return close;
    }
    else if (pt.IsCubicBezier()){
      PyObject* bezier = PyList_New(7);
      PyList_SetItem(bezier, 0, PyUnicode_FromString("C"));
      PyList_SetItem(bezier, 1, PyFloat_FromDouble(pt.p.x));
      PyList_SetItem(bezier, 2, PyFloat_FromDouble(pt.p.y));
      PyList_SetItem(bezier, 3, PyFloat_FromDouble(pt.c.x));
      PyList_SetItem(bezier, 4, PyFloat_FromDouble(pt.c.y));
      PyList_SetItem(bezier, 5, PyFloat_FromDouble(pt.d.x));
      PyList_SetItem(bezier, 6, PyFloat_FromDouble(pt.d.y));
      return bezier;
    }
    else if (pt.IsLine()){
      PyObject* lineTo = PyList_New(3);
      PyList_SetItem(lineTo, 0, PyUnicode_FromString("L"));
      PyList_SetItem(lineTo, 1, PyFloat_FromDouble(pt.p.x));
      PyList_SetItem(lineTo, 2, PyFloat_FromDouble(pt.p.y));
      return lineTo;
    }
    else if (pt.IsMove()){
      PyObject* moveTo = PyList_New(3);
      PyList_SetItem(moveTo, 0, PyUnicode_FromString("M"));
      PyList_SetItem(moveTo, 1, PyFloat_FromDouble(pt.p.x));
      PyList_SetItem(moveTo, 2, PyFloat_FromDouble(pt.p.y));
      return moveTo;
    }
    assert(false);
    return nullptr;
}

PyObject* build_result(const Point& pos){
  return Py_BuildValue("dd", pos.x, pos.y);
}

PyObject* build_result(PyObject* obj){
  return obj;
}

PyObject* build_result(const Radii& r){
  return Py_BuildValue("dd", r.x, r.y);
}

PyObject* build_result(const Rect& rect){
  return Py_BuildValue("dddd", rect.x, rect.y, rect.w, rect.h);
}

PyObject* build_result(const Settings& s){
  return pythoned(s);
}

PyObject* build_result(const std::string& str){
  return PyBytes_FromStringAndSize(str.c_str(), resigned(str.size()));
}

PyObject* build_result(const Tri& tri){
  return pythoned(tri);
}

PyObject* build_result(const utf8_string& s){
  return build_unicode(s);
}

} // namespace
