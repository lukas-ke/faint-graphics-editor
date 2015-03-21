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

#include <sstream>
#include "bitmap/bitmap.hh"
#include "bitmap/gradient.hh"
#include "geo/int-point.hh"
#include "geo/limits.hh"
#include "geo/pathpt.hh"
#include "text/formatting.hh"
#include "python/py-include.hh"
#include "python/py-util.hh"

namespace faint{

using ensure_coord_double = std::is_same<coord, double>;

Optional<utf8_string> parse_py_unicode(PyObject* obj){
  scoped_ref utf8(PyUnicode_AsUTF8String(obj));
  if (utf8 == nullptr){
    // PyUnicode_AsUTF8String will have raised TypeError
    return no_option();
  }
  const char* str = PyBytes_AsString(utf8.get());
  if (str == nullptr){
    // PyBytes_AsString will have raised TypeError
    return no_option();
  }
  return option(utf8_string(str));
}

PyObject* build_unicode(const utf8_string& str){
  return PyUnicode_DecodeUTF8(str.c_str(), resigned(str.bytes()), nullptr);
}

bool invalid_color(int r, int g, int b, int a){
  if (!(0 <= r && r <= 255)){
    PyErr_SetString(PyExc_ValueError,
      "Invalid rgba-color: Red component out of range (0-255).");
    return true;
  }
  else if (!(0 <= g && g <= 255)){
    PyErr_SetString(PyExc_ValueError,
      "Invalid rgba-color: Green component out of range (0-255).");
    return true;
  }
  else if (!(0 <= b && b <= 255)){
    PyErr_SetString(PyExc_ValueError,
      "Invalid rgba-color: Blue component out of range (0-255).");
    return true;
  }
  else if (!(0 <= a && a <= 255)){
    PyErr_SetString(PyExc_ValueError,
      "Invalid rgba-color: Alpha component out of range (0-255).");
    return true;
  }

  // Color is valid
  return false;
}

bool invalid_pixel_pos(int x, int y, const Bitmap& bmp){
  return invalid_pixel_pos(IntPoint(x,y), bmp);
}

bool invalid_pixel_pos(const IntPoint& pos, const Bitmap& bmp){
  if (!point_in_bitmap(bmp, pos)){
    PyErr_SetString(PyExc_ValueError, "Point outside image");
    return true;
  }
  return false;
}

static utf8_string object_string(PyObject* obj){
  scoped_ref pyStr(PyObject_Str(obj));
  if (pyStr == nullptr){
    return "";
  }
  scoped_ref utf8(PyUnicode_AsUTF8String(pyStr.get()));
  assert(utf8 != nullptr);
  const char* bytes = PyBytes_AsString(utf8.get());
  if (bytes == nullptr){
    return "";
  }

  return utf8_string(bytes);
}

bool parse_color(PyObject* args, Color& color, bool allowAlpha){
  int r = 255;
  int g = 255;
  int b = 255;
  int a = 255;

  bool ok = PyArg_ParseTuple(args, "iiii", &r, &g, &b, &a) ||
    PyArg_ParseTuple(args, "iii", &r, &g, &b) ||
    PyArg_ParseTuple(args, "(iiii)", &r, &g, &b, &a) ||
    PyArg_ParseTuple(args, "(iii)", &r, &g, &b);

  if (!ok){
    PyErr_SetString(PyExc_TypeError,
      "Invalid color specification. Valid formats for colors are "
      "r,g,b and r,g,b,a");
    return false;
  }
  PyErr_Clear();

  if (invalid_color(r, g, b, a)){
    return false;
  }

  if (!allowAlpha && a != 255){
    PyErr_SetString(PyExc_ValueError,
      "Alpha not supported by this function");
    return false;
  }

  color = color_from_ints(r,g,b,a);
  return true;
}

bool parse_color_stop(PyObject* obj, ColorStop& stop){
  if (PySequence_Length(obj) != 2){
    PyErr_SetString(PyExc_ValueError,
      "Color stop must be specified using offset,(r,g,b[,a])");
    return false;
  }

  scoped_ref pyOffset(PySequence_GetItem(obj, 0));
  if (!PyFloat_Check(pyOffset.get())){
    PyErr_SetString(PyExc_ValueError,
      "Color stop must start with a floating point offset");
    return false;
  }

  double offset = PyFloat_AsDouble(pyOffset.get());
  scoped_ref pyColor(PySequence_GetItem(obj,1));
  Color c;
  if (!parse_color(pyColor.get(), c)){
    return false;
  }

  stop = ColorStop(c, offset);
  return true;
}

static std::istream& operator>>(std::istream& s, Point& pt){
  return s >> pt.x >> pt.y;
}

static std::istream& operator>>(std::istream& s, Radii& r){
  return s >> r.x >> r.y;
}

// Fixme: Maybe make this more svg-grammar-compliant
std::vector<PathPt> parse_svg_path(const std::string& s){
  std::stringstream ss(s);

  char controlChar;
  Point current;
  std::vector<PathPt> points;
  bool prevQuadratic = false;
  bool prevCubic = false;
  Point prev;

  while (ss >> controlChar){
    if (controlChar == 'M'){ // Move to absolute
      prevCubic = prevQuadratic = false;
      Point pt;
      if (ss >> pt){
        current = pt;
        points.push_back(PathPt::MoveTo(pt));
        // Absolute line-to coordinates may follow
        while (ss >> pt){
          current = pt;
          points.push_back(PathPt::LineTo(pt));
        }
      }
      ss.clear();
    }
    else if (controlChar == 'm'){ // Move to relative
      prevCubic = prevQuadratic = false;
      Point pt;
      if (ss >> pt){
        current += pt;
        points.push_back(PathPt::MoveTo(current));

        while (ss >> pt){
          current += pt;
          points.push_back(PathPt::LineTo(current));
        }
      }
      ss.clear();
    }
    else if (controlChar == 'L'){ // Line-to absolute
      prevCubic = prevQuadratic = false;
      Point pt;
      while (ss >> pt){
        current = pt;
        points.push_back(PathPt::LineTo(pt));
      }
      ss.clear();
    }
    else if (controlChar == 'l'){ // Line-to relative
      prevCubic = prevQuadratic = false;
      Point pt;
      while(ss >> pt){
        current += pt;
        points.push_back(PathPt::LineTo(current));
      }
      ss.clear();
    }
    else if (controlChar == 'V'){ // Vertical line-to absolute
      prevCubic = prevQuadratic = false;
      coord y;
      while (ss >> y){
        current.y = y;
        points.push_back(PathPt::LineTo(current));
      }
      ss.clear();
    }
    else if (controlChar == 'v'){ // Vertical line-to relative
      prevCubic = prevQuadratic = false;
      coord dy;
      while (ss >> dy){
        current.y += dy;
        points.push_back(PathPt::LineTo(current));
      }
      ss.clear();
    }
    else if (controlChar == 'H'){ // Horizontal line to absolute
      prevCubic = prevQuadratic = false;
      coord x;
      while (ss >> x){
        current.x = x;
        points.push_back(PathPt::LineTo(current));
      }
      ss.clear();
    }
    else if (controlChar == 'h'){ // Horizontal line to relative
      prevCubic = prevQuadratic = false;
      coord dx;
      while (ss >> dx){
        current.x += dx;
        points.push_back(PathPt::LineTo(current));
      }
      ss.clear();
    }
    else if (controlChar == 'C'){ // Absolute cubic bezier
      prevQuadratic = false;
      Point p0, p1, p2;
      while (ss >> p0 >> p1 >> p2){
        current = p2;
        points.push_back(PathPt::CubicBezierTo(p2, p0, p1));
      }
      prevCubic = true;
      prev = p1;
      ss.clear();
    }
    else if (controlChar == 'c'){ // Relative cubic bezier
      prevQuadratic = false;
      Point p0, p1, p2;
      while (ss >> p0 >> p1 >> p2){
        p0 += current;
        p1 += current;
        p2 += current;
        points.push_back(PathPt::CubicBezierTo(p2, p0, p1));
        current = p2;
      }
      prevCubic = true;
      prev = p1;
      ss.clear();
    }
    else if (controlChar == 'S'){ // Absolute short hand cubic
      prevQuadratic = false;
      Point p1, p2;
      while (ss >> p1 >> p2){
        Point p0 = prevCubic ? 2 * current - prev : current;

        points.push_back(PathPt::CubicBezierTo(p2, p0, p1));
        current = p2;

        prevCubic = true;
        prev = p1;
      }
      ss.clear();
    }
    else if (controlChar == 's'){ // Relative short hand cubic
      prevQuadratic = false;
      Point p1, p2;
      while (ss >> p1 >> p2){
        Point p0 = prevCubic ? 2 * current - prev : current;
        p1 += current;
        p2 += current;
        points.push_back(PathPt::CubicBezierTo(p2, p0, p1));

        current = p2;
        prevCubic = true;
        prev = p1;
      }
      ss.clear();
    }
    else if (controlChar == 'Q'){ // Absolute quadratic bezier
      prevCubic = false;
      Point p0, p1;
      while (ss >> p0 >> p1){
        // Convert to cubic bezier
        points.push_back(PathPt::CubicBezierTo(p1,
            current + 2.0/3.0 * (p0 - current),
          p1 + 2.0/3.0 * (p0 - p1)));
        current = p1;
      }
      prevQuadratic = true;
      prev = p0;
      ss.clear();
    }
    else if (controlChar == 'q'){ // Relative quadratic bezier
      prevCubic = false;
      Point p0, p1;
      while (ss >> p0 >> p1){
        // Convert to cubic bezier
        p0 += current;
        p1 += current;

        points.push_back(PathPt::CubicBezierTo(p1,
          current + 2.0/3.0 * (p0 - current),
          p1 + 2.0/3.0 * (p0 - p1)));
        current = p1;
      }
      prevQuadratic = true;
      prev = p0;
      ss.clear();
    }
    else if (controlChar == 'T'){ // Absolute short hand quadratic
      prevCubic = false;
      Point p1;
      while (ss >> p1){
        // Convert to cubic bezier
        Point p0 = prevQuadratic ? 2 * current - prev : current;

        points.push_back(PathPt::CubicBezierTo(p1,
            current + 2.0/3.0*(p0 - current),
            p1 + 2.0/3.0*(p0 - p1)));

        current = p1;
        prevQuadratic = true;
        prev = p0;
      }
      ss.clear();
    }
    else if (controlChar == 't'){ // Relative short hand quadratic
      prevCubic = false;
      Point p1;
      while (ss >> p1){
        Point p0 = prevQuadratic ? 2 * current - prev : current;
        p1 += current;
        points.push_back(PathPt::CubicBezierTo(p1,
            current + 2.0 / 3.0 * (p0 - current),
            p1 + 2.0/3.0 * (p0 - p1)));

        current = p1;
        prevQuadratic = true;
        prev = p0;
      }
      ss.clear();
    }
    else if (controlChar == 'z' || controlChar == 'Z'){ // Close path
      prevQuadratic = prevCubic = false;
      points.push_back(PathPt::PathCloser());
    }
    else if (controlChar == 'A'){ // Absolute Arc to
      prevQuadratic = prevCubic = false;
      coord xAxisRotation;
      Radii r;
      Point pt;
      int largeArcFlag;
      int sweepFlag;
      while (ss >> r >> xAxisRotation >> largeArcFlag >> sweepFlag >> pt){
        points.push_back(PathPt::Arc(r,
          Angle::Deg(xAxisRotation),
          largeArcFlag,
          sweepFlag,
          pt));
        current = pt;
      }
      ss.clear();
    }
    else if (controlChar == 'a'){ // Relative arc to
      prevQuadratic = prevCubic = false;
      coord xAxisRotation;
      Point pt;
      Radii r;
      int largeArcFlag;
      int sweepFlag;
      while (ss >> r >> xAxisRotation >> largeArcFlag >> sweepFlag >> pt){
        points.push_back(PathPt::Arc(r, Angle::Deg(xAxisRotation),
          largeArcFlag,
          sweepFlag,
          current + pt));
        current += pt;
      }
      ss.clear();
    }
  }
  return points;
}

static utf8_string get_repr(const Color& c){
  return bracketed(str_rgba(c));
}

static utf8_string get_repr(const Point& pt){
  std::stringstream ss;
  ss << "(" << pt.x << ", " << pt.y << ")";
  return utf8_string(ss.str());
}

static utf8_string get_repr(const ColorStop& stop){
  std::stringstream ss;
  ss << stop.GetOffset() << ", " << get_repr(stop.GetColor());
  return bracketed(utf8_string(ss.str()));
}

static utf8_string get_repr(const color_stops_t& stops){
  std::stringstream ss;
  ss << "[";
  for (color_stops_t::const_iterator it = begin(stops); it != end(stops); ++it){
    ss << get_repr(*it);
    if (it != stops.end() - 1){
      ss << ",";
    }
  }
  ss << "]";
  return utf8_string(ss.str());
}

utf8_string get_repr(const LinearGradient& g){
  std::stringstream ss;
  ss << "LinearGradient(" << g.GetAngle().Rad() << ", " <<
    get_repr(g.GetStops()) << ")";
  return utf8_string(ss.str());
}

utf8_string get_repr(const RadialGradient& g){
  std::stringstream ss;
  ss << "RadialGradient(" << get_repr(g.GetCenter()) << ", " <<
    get_repr(g.GetStops()) << ")";
  return utf8_string(ss.str());
}

bool py_error_occurred(){
  // PyErr_Occurred returns a borrowed reference
  // to the exception type (or nullptr if none).
  return PyErr_Occurred() != nullptr;
}

bool py_load_error_occurred(){
  PyObject* errType = PyErr_Occurred();
  if (errType == nullptr){
    return false;
  }
  PyObject* loadError = get_load_exception_type();
  int result = PyErr_GivenExceptionMatches(errType, loadError);
  return result != 0;
}

bool py_save_error_occurred(){
  PyObject* errType = PyErr_Occurred();
  if (errType == nullptr){
    return false;
  }
  PyObject* saveError = get_save_exception_type();
  int result = PyErr_GivenExceptionMatches(errType, saveError);
  return result != 0;
}

utf8_string py_error_string(){
  PyObject* type = nullptr;
  PyObject* value = nullptr;
  PyObject* traceBack = nullptr;

  PyErr_Fetch(&type, &value, &traceBack);
  if (value == nullptr){
    py_xdecref(type);
    py_xdecref(traceBack);
    return utf8_string("");
  }

  scoped_ref exceptionStr(PyObject_Str(value));
  py_xdecref(type);
  py_xdecref(value);
  py_xdecref(traceBack);

  if (exceptionStr == nullptr){
    return utf8_string("");
  }

  scoped_ref utf8(PyUnicode_AsUTF8String(exceptionStr.get()));
  const char* str(PyBytes_AsString(utf8.get()));
  utf8_string errorString(str);
  return errorString;
}

std::vector<utf8_string> parse_traceback(PyObject* tb){
  scoped_ref tracebackModule(PyImport_ImportModule("traceback"));
  assert(tracebackModule != nullptr);

  PyObject* dict = PyModule_GetDict(tracebackModule.get()); // Borrowed
  PyObject* format_tb = PyDict_GetItemString(dict, "format_tb"); // Borrowed

  const int maxTracebackItems = 20;
  PyObject* args = Py_BuildValue("Oi", tb, maxTracebackItems);
  PyObject* tbList = PyEval_CallObject(format_tb, args);
  assert(PySequence_Check(tbList));
  auto n = PySequence_Length(tbList);
  assert(n >= 0);

  std::vector<utf8_string> v;
  v.reserve(static_cast<size_t>(n));
  for (Py_ssize_t i = 0; i != n; i++){
    scoped_ref item(PySequence_GetItem(tbList, i));
    v.push_back(object_string(item.get()));
  }

  return v;
}

static utf8_string get_name(PyObject* o){
  scoped_ref nameStr(PyObject_GetAttrString(o, "__name__"));
  if (nameStr == nullptr){
    return "";
  }
  else{
    return parse_py_unicode(nameStr.get()).Or("");
  }
}

static utf8_string decode_or_die(PyObject* obj){
  scoped_ref pyBytes(PyUnicode_AsUTF8String(obj));
  assert(pyBytes != nullptr);

  const char* utf8 = PyBytes_AsString(pyBytes.get());
  assert(utf8 != nullptr);

  return utf8_string(utf8);
}

static void decode_syntax_error_tuple(PyObject* tuple, FaintPyExc& info){
  auto numItems = PySequence_Length(tuple);
  if (numItems == 0){
    return;
  }
  scoped_ref message(PySequence_GetItem(tuple, 0));
  if (message != nullptr){
    if (PyUnicode_Check(message.get())){
      info.message = decode_or_die(message.get());
    }
  }
  if (numItems == 1){
    return;
  }
  scoped_ref syntaxErrTuple(PySequence_GetItem(tuple,1));
  if (!PyTuple_Check(syntaxErrTuple.get())){
    return;
  }

  auto numExtra = PySequence_Length(syntaxErrTuple.get());
  if (numExtra == 0){
    return;
  }
  FaintPySyntaxError syntaxError;

  scoped_ref fileName(PySequence_GetItem(syntaxErrTuple.get(), 0));
  if (fileName != nullptr && PyUnicode_Check(fileName.get())){
    syntaxError.file = decode_or_die(fileName.get());
  }
  if (numExtra > 1){
    scoped_ref line(PySequence_GetItem(syntaxErrTuple.get(), 1));
    if (line != nullptr && PyLong_Check(line.get())){
      syntaxError.line = as_int(line.get()).Get();
    }
    if (numExtra > 2){
      scoped_ref column(PySequence_GetItem(syntaxErrTuple.get(), 2));
      if (column != nullptr && PyLong_Check(column.get())){
        syntaxError.col = as_int(column.get()).Get();
      }

      if (numExtra > 3){
        scoped_ref code(PySequence_GetItem(syntaxErrTuple.get(), 3));
        if (code != nullptr && PyUnicode_Check(code.get())){
          syntaxError.code = decode_or_die(code.get());
        }
      }
    }
  }
  info.syntaxErrorInfo.Set(syntaxError);
}

void decode_syntax_error_object(PyObject* error, FaintPyExc& info){
  scoped_ref msg(PyObject_GetAttrString(error, "msg"));
  scoped_ref lineNo(PyObject_GetAttrString(error, "lineno"));
  scoped_ref offset(PyObject_GetAttrString(error, "offset"));
  scoped_ref text(PyObject_GetAttrString(error, "text"));
  scoped_ref filename(PyObject_GetAttrString(error, "filename"));

  FaintPySyntaxError errInfo;
  if (msg != nullptr && PyUnicode_Check(msg.get())){
    info.message = decode_or_die(msg.get());
  }
  if (lineNo != nullptr && PyLong_Check(lineNo.get())){
    errInfo.line = as_int(lineNo.get()).Get();
  }
  if (offset != nullptr && PyLong_Check(offset.get())){
    errInfo.col = as_int(offset.get()).Get();
  }
  if (text != nullptr && PyUnicode_Check(text.get())){
    errInfo.code = decode_or_die(text.get());
  }
  if (filename != nullptr && PyUnicode_Check(filename.get())){
    errInfo.file = decode_or_die(filename.get());
  }
  info.syntaxErrorInfo.Set(errInfo);
}

FaintPyExc py_error_info(){
  PyObject* type = nullptr;
  PyObject* value = nullptr;
  PyObject* traceback = nullptr;
  PyErr_Fetch(&type, &value, &traceback);

  FaintPyExc info;
  if (type != nullptr){
    info.type = get_name(type);
  }
  if (value != nullptr){
    if (PyUnicode_Check(value)){
      info.message = decode_or_die(value);
    }
    if ((PySequence_Check(value) && info.type == "SyntaxError") ||
      info.type == "IndentationError"){

      // When the Syntax error is in executed text, it's a tuple
      // rather than an exception object.
      decode_syntax_error_tuple(value, info);
    }
    else if (info.type == "SyntaxError"){
      decode_syntax_error_object(value, info);
    }
    else{
      info.message = object_string(value);
    }
  }
  if (traceback != nullptr){
    info.stackTrace = parse_traceback(traceback);
  }
  return info;
}

PyObject* get_load_exception_type(){
  static PyObject* faintPyLoadError = PyErr_NewException("ifaint.LoadError",
    nullptr, nullptr);
  return faintPyLoadError;
}

PyObject* get_save_exception_type(){
  static PyObject* faintPySaveError = PyErr_NewException("ifaint.SaveError",
    nullptr, nullptr);
  return faintPySaveError;
}

Optional<int> as_int(PyObject* obj){
  auto v = PyLong_AsLong(obj);
  if (v == -1 && PyErr_Occurred()){
    return {};
  }
  return can_represent<int>(v) ?
    option(static_cast<int>(v)) : no_option();
}

} // namespace
