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
#include "python/py-include.hh"
#include "app/canvas.hh"
#include "app/frame.hh"
#include "bitmap/draw.hh"
#include "bitmap/pattern.hh"
#include "formats/png/file-png.hh"
#include "geo/pathpt.hh"
#include "geo/pathpt-iter.hh"
#include "geo/offsat.hh"
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
#include "text/slice.hh"
#include "tools/tool-id.hh"
#include "util/enum-util.hh"
#include "util/image.hh"
#include "util/index-iter.hh"
#include "util/make-vector.hh"
#include "util/optional.hh"
#include "util/settings.hh"
#include "util-wx/encode-bitmap.hh"
#include "util-wx/file-path-util.hh"
#include "util-wx/font.hh"
#include "generated/python/settings/setting-functions.hh"
#include "python/py-ugly-forward.hh"
#include "python/py-parse.hh"
#include "python/py-functions.hh"

namespace faint{

/* function: "blit(src, (x,y), dst)\n
Blits src onto dst with the top left corner of src at (x,y)."
name: "blit" */
static void py_blit_bitmap(const Bitmap& src,
  const IntPoint& topLeft,
  bitmapObject*& dst)
{
  blit(offsat(src, topLeft), onto(dst->bmp));
}

// Helper for to_svg_path
static utf8_string points_to_svg_path_string(const std::vector<PathPt>& points){
  std::stringstream ss;

  for_each_pt(points,
    [&ss](const ArcTo& a){
      ss << "A" << " " <<
        a.r.x << " " <<
        a.r.y << " " <<
        a.axisRotation.Deg() << " " <<
        a.largeArcFlag << " " <<
        a.sweepFlag << " " <<
        a.p.x << " " <<
        a.p.y << " ";
    },
    [&ss](const Close&){
      ss << "z ";
    },
    [&ss](const CubicBezier& b){
      ss << "C " <<
        b.c.x << " " << b.c.y << " " <<
        b.d.x << " " << b.d.y << " " <<
        b.p.x << " " << b.p.y << " ";
    },
    [&ss](const LineTo& l){
      ss << "L " <<
        l.p.x << " " << l.p.y << " ";
    },
    [&ss](const MoveTo& m){
      ss << "M " <<
        m.p.x << " " << m.p.y << " ";
    });

  return utf8_string(slice_up_to(ss.str(), -1));
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

  scoped_ref pngStrPy(PySequence_GetItem(args, 0));
  if (!PyBytes_Check(pngStrPy.get())){
    PyErr_SetString(PyExc_TypeError, "Invalid png-string.");
    return false;
  }

  auto len = PyBytes_Size(pngStrPy.get());
  if (len <= 0){
    return false;
  }

  const char* bytes = PyBytes_AsString(pngStrPy.get());
  if (bytes == nullptr){
    return false;
  }

  Bitmap bmp(from_png(bytes, static_cast<size_t>(len)));
  if (bitmap_ok(bmp)){
    out = bmp;
    return true;
  }
  else{
    return false;
  }
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

  scoped_ref jpgStrPy(PySequence_GetItem(args, 0));
  if (!PyBytes_Check(jpgStrPy.get())){
    PyErr_SetString(PyExc_TypeError, "Invalid jpg-string.");
    return false;
  }

  auto len = PyBytes_Size(jpgStrPy.get());
  if (len <= 0){
    return false;
  }

  const char* bytes = PyBytes_AsString(jpgStrPy.get());
  if (bytes == 0){
    return false;
  }

  Bitmap bmp(from_jpg(bytes, static_cast<size_t>(len)));
  if (bitmap_ok(bmp)){
    out = bmp;
    return true;
  }
  else{
    return false;
  }
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
  return Frame(*obj.ctx, *obj.canvas, obj.canvas->GetFrame(obj.frameId));
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

/* function: "write_png(bmp, path[, color_type, text_dict])\n
Writes the bitmap as a png at the given path.\n
Throws OSError on failure.\n
\n
Optional parameters:\n
 - color_type: A constant from the png-module (e.g. png.RGB_ALPHA)\n
 - text_dict: A dictionary of key-strings to value-strings\n
   for png tEXt meta-data."
name: "write_png" */
static void write_png_py(const Bitmap& bmp,
  const FilePath& p,
  const Optional<int>& rawColorType,
  const Optional<png_tEXt_map>& maybeTextChunks)
{
  const auto defaultType = static_cast<int>(PngColorType::RGB_ALPHA);

  const auto colorType =
    to_enum<PngColorType>(rawColorType.Or(defaultType)).Visit(
      [](const PngColorType t){
        return t;
      },
      []() -> PngColorType{
        throw ValueError("color_type out of range.");
      });

  auto r = maybeTextChunks.Visit(
    [&](const png_tEXt_map& textChunks){
      return write_png(p, bmp, colorType, textChunks);
    },
    [&](){
      return write_png(p, bmp, colorType);
    });

  if (!r.Successful()){
    throw OSError(r.ErrorDescription());
  }
}

using png_pair = std::pair<Bitmap, png_tEXt_map>;

/* function: "read_png(path)->(bmp, text_dict)\n
Reads the png-file at path, returning a Bitmap and a
dictionary of tEXt-entries.\n
Throws OSError on failure."
name: "read_png" */
static png_pair read_png_py(const FilePath& path){
  return read_png_meta(path).Visit(
    [](const Bitmap_and_tEXt& obj) -> png_pair{
      return std::make_pair(obj.bmp, obj.text);
    },
    [](const utf8_string& error) -> png_pair{
      throw OSError(error);
    });
}

/* extra_include: "generated/python/settings/setting-function-defs.hh" */

#include "generated/python/method-def/py-functions-methoddef.hh"

PyMethodDef* get_py_functions(){
  return faint_interface_methods;
}

} // namespace
