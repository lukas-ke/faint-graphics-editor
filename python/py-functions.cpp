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
#include "formats/png/file-png.hh"
#include "geo/pathpt.hh"
#include "geo/rect.hh"
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
#include "python/py-ugly-forward.hh"
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

/* function: "_one_color_bg(frame)->b\n
True if the frame background consists of a single color."
name: "_one_color_bg" */
static bool one_color_bg(const Image* image){
  return image->GetBackground().Visit(
    [](const Bitmap& bmp){
      return is_blank(bmp);
    },
    [](const ColorSpan&){
      return true;
    });
}

/* function: "bitmap_from_png_string(s)->bmp\n
Creates a bitmap from the PNG string." */
static Bitmap bitmap_from_png_string(const std::string& bytes){
  if (bytes.size() == 0){
    throw ValueError("Empty string");
  }
  return or_throw<ValueError>(from_png(bytes.c_str(), bytes.size()));
}

/* function: "bitmap_from_jpg_string(s)->bmp\n
Creates a bitmap from the JPG string." */
static Bitmap bitmap_from_jpg_string(const std::string& bytes){
  if (bytes.size() == 0){
    throw ValueError("Empty string");
  }
  return or_throw<ValueError>(from_jpg(bytes.c_str(), bytes.size()));
}

/* function: "encode_bitmap_png(bmp)->bytes\n
Returns a Bytes object with with the Bitmap encoded in PNG." */
static std::string encode_bitmap_png(const Bitmap& bmp){
  return to_png_string(bmp);
}

/* function: "get_object_frame()\n
Returns the frame containing the passed in object." */
static Frame get_object_frame(const BoundObject<Object>& obj){
  return Frame(*obj.ctx, *obj.canvas, obj.canvas->GetFrame(obj.frameId));
}

/* function: "write_png(bmp, path[, color_type, text_dict])\n
Writes the bitmap as a png at the given path.\n
Raises OSError on failure.\n
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
Raises OSError on failure."
name: "read_png" */
static png_pair read_png_py(const FilePath& path){
  return read_png_meta(path).Visit(
    [](const Bitmap_and_tEXt& obj) -> png_pair{
      return {obj.bmp, obj.text};
    },
    [](const utf8_string& error) -> png_pair{
      throw OSError(error);
    });
}

/* function: "create_Rect(...)\n
Temporary helper for Shape/Pimage." */
extern PyObject* create_Rect(const Rect&, const Optional<Settings>&);

/* function: "create_Path(...)\n
Temporary helper for Shape/Pimage." */
extern PyObject* create_Path(const utf8_string&, const Optional<Settings>&);

/* function: "create_Ellipse(...)\n
Temporary helper for Shape/Pimage." */
extern PyObject* create_Ellipse(const Rect&, const Optional<Settings>&);

/* function: "create_Group(...)\n
Temporary helper for Shape/Pimage." */
extern PyObject* create_Group(PyObject*);

/* function: "create_Line(...)\n
Temporary helper for Shape/Pimage." */
extern PyObject* create_Line(const std::vector<coord>&,
  const Optional<Settings>&);

/* function: "create_Polygon()\n
Temporary helper for Shape/Pimage." */
extern PyObject* create_Polygon(const std::vector<coord>&,
  const Optional<Settings>&);

/* function: "create_Raster((x,y[,w,h]), Bitmap[, settings])\n
Adds a Raster object, scaled to the specified rectangle." */
extern PyObject* create_Raster(const Either<Point, Rect>& region,
  const Bitmap&,
  const Optional<Settings>&);

/* function: "create_Spline((x0,y0,x1,y1,...), settings)->Spline\n
Adds a Spline object. The points are a list of coordinates sort-of
followed by the spline.\n\nFor more precise control-point handling use
Path instead." */
extern PyObject* create_Spline(const std::vector<coord>&,
  const Optional<Settings>&);

/* function: "create_Text(pos|rect, str[, settings])->Text\n
Creates a Text object. If a rectangle is given as the first argument,
the text is bounded. " */
extern PyObject* create_Text(const Either<Rect, Point>& region,
  const utf8_string&,
  const Optional<Settings>&);

#include "generated/python/method-def/py-functions-method-def.hh"

PyMethodDef* get_py_functions(){
  return faint_interface_methods;
}

} // namespace
