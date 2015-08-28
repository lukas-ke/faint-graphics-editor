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

#include "geo/geo-func.hh"
#include "geo/points.hh"
#include "geo/rect.hh"
#include "geo/size.hh"
#include "geo/tri.hh"
#include "objects/object.hh"
#include "objects/objellipse.hh"
#include "objects/objline.hh"
#include "objects/objpolygon.hh"
#include "objects/objraster.hh"
#include "objects/objrectangle.hh"
#include "objects/objspline.hh"
#include "objects/objtext.hh"
#include "util/default-settings.hh"
#include "util/object-util.hh"
#include "python/mapped-type.hh"
#include "python/py-shape.hh"
#include "python/py-ugly-forward.hh"
#include "python/py-util.hh"
#include "python/py-add-type-object.hh"
#include "util/setting-id.hh"
#include "python/py-shape-properties.hh"
#include "generated/python/settings/shape-properties.hh"

namespace faint{

class Object;
class Canvas;

extern PyTypeObject ShapeType;

struct shapeObject{
  PyObject_HEAD
  Object* obj;
};

template<>
struct MappedType<Object&>{
  using PYTHON_TYPE = shapeObject;

  static Object& GetCppObject(shapeObject* self){
    return *self->obj;
  }

  static bool Expired(shapeObject*){
    // Does not expire (unless reference count reaches 0)
    return false;
  }

  static void ShowError(shapeObject*){
    // Does not expire (unless reference count reaches 0)
  }
};

template<>
struct MappedType<const Object&>{
  using PYTHON_TYPE = shapeObject;

  static const Object& GetCppObject(shapeObject* self){
    return *self->obj;
  }

  static bool Expired(shapeObject*){
    // Does not expire (unless reference count reaches 0)
    return false;
  }

  static void ShowError(shapeObject*){
    // Does not expire (unless reference count reaches 0)
  }
};

/* method: "__copy__(self) Used by copy.copy"
name: "__copy__" */
static PyObject* Shape_copy(Object& self){
  shapeObject* cp =
    (shapeObject*)ShapeType.tp_alloc(&ShapeType, 0);
  cp->obj = self.Clone();
  return (PyObject*)(cp);
}

static PyObject* Shape_repr(shapeObject* self){
  return build_unicode(self->obj->GetType());
}

static PyObject* Shape_richcompare(shapeObject* self, PyObject* otherRaw, int op){
  if (!PyObject_IsInstance(otherRaw, (PyObject*)&ShapeType)){
    Py_RETURN_NOTIMPLEMENTED;
  }
  shapeObject* other((shapeObject*)otherRaw);
  return py_rich_compare(self->obj->GetId(), other->obj->GetId(), op);
}

static void Shape_dealloc(shapeObject* self){
  delete self->obj;
  self->obj = nullptr;
  // Fixme: Call some base-dealloc? (cw py-bitmap.cpp)
}

/* method: "get_points()->(x0,y0,x1,y1,...)\nReturns a list of
vertices for Polygons, Splines and Paths. Returns the points in the
Tri for other objects." */
static std::vector<coord> Shape_get_points(const Object& self){
  return get_flat_coordinate_list(self);
}

/* method: "get_settings()->Settings\n
Returns a copy of this object's settings." */
static Settings Shape_get_settings(const Object& self){
  return self.GetSettings();
}

/* method: "get_type()->s\n
Returns the type of the object, as a string." */
static utf8_string Smth_get_type(const Object& self){
  return self.GetType();
}

/* method: "get_tri()" */
static Tri Shape_get_tri(Object& self){
  return self.GetTri();
}

/* method: "get_tri()" */
static void Shape_set_tri(Object& self, const Tri& tri){
  return self.SetTri(tri);
}

/* method: "get_rect() -> (x,y,w,h)\n
Returns the bounding rectangle. " */
static Rect Shape_rect(Object& self){
  return bounding_rect(self.GetTri());
}

static void Shape_init(shapeObject& self){
  self.obj = nullptr;
  throw TypeError("Shape can not be created manually. "
    "Use the object creation functions (e.g. create_Rect) instead.");
}

static PyObject* Shape_new(PyTypeObject* type, PyObject*, PyObject*){
  shapeObject* self;
  self = (shapeObject*)type->tp_alloc(type, 0);
  return (PyObject*)self;
}

#include "generated/python/method-def/py-shape-methoddef.hh"

PyTypeObject ShapeType = {
  PyVarObject_HEAD_INIT(nullptr,0)
  "Shape", // tp_name
  sizeof(shapeObject), // tp_basicsize
  0, // tp_itemsize
  (destructor)Shape_dealloc, // tp_dealloc
  nullptr, // tp_print
  nullptr, // tp_getattr
  nullptr, // tp_setattr
  nullptr, // tp_compare
  (reprfunc)Shape_repr, // tp_repr
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
  "", // tp_doc
  nullptr, // tp_traverse
  nullptr, // tp_clear
  (richcmpfunc)Shape_richcompare, // tp_richcompare
  0, // tp_weaklistoffset
  nullptr, // tp_iter
  nullptr, // tp_iternext
  Shape_methods, // tp_methods
  nullptr, // tp_members
  Shape_properties, // tp_getset
  nullptr, // tp_base
  nullptr, // tp_dict
  nullptr, // tp_descr_get
  nullptr, // tp_descr_set
  0, // tp_dictoffset
  INIT_FORWARDER(Shape_init), // tp_init
  nullptr, // tp_alloc
  Shape_new, // tp_new
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

void add_type_Shape(PyObject* module){
  add_type_object(module, ShapeType, "Shape");
}

PyObject* create_Shape(Object* obj){
  shapeObject* shape = (shapeObject*)ShapeType.tp_alloc(&ShapeType, 0);
  shape->obj = obj;
  return (PyObject*)shape;
}

PyObject* create_Rect(const Rect& r, const Optional<Settings>& maybeSettings){
  const auto tri = tri_from_rect(r);
  const auto s = maybeSettings.Or(default_rectangle_settings());
  return create_Shape(create_rectangle_object(tri, s));
}

PyObject* create_Ellipse(const Rect& r, const Optional<Settings>& maybeSettings){
  const auto tri = tri_from_rect(r);
  const auto s = maybeSettings.Or(default_ellipse_settings());
  return create_Shape(create_ellipse_object(tri, s));
}

PyObject* create_Group(PyObject*){
  return nullptr; // FIXME
}

  // Fixme: Path

PyObject* create_Line(const std::vector<coord>& coords,
  const Optional<Settings>& maybeSettings)
{
  if (coords.size() < 4){
    throw ValueError("At least four coordintes required for line"
      " (x0, y0, x1, y1).");
  }
  else if (coords.size() % 2 != 0){
    throw ValueError("Number of coordinates must be an even number.");
  }

  const auto s = maybeSettings.Or(default_line_settings());
  return create_Shape(create_line_object(points_from_coords(coords), s));
}

PyObject* create_Polygon(const std::vector<coord>& coords,
  const Optional<Settings>& maybeSettings)
{
  const size_t n = coords.size();
  if (n == 0){
    throw ValueError("No points specified.");
  }
  else if (n % 2 != 0){
    throw ValueError("Uneven number of coordinates.");
  }

  const auto s = maybeSettings.Or(default_polygon_settings());
  return create_Shape(create_polygon_object(points_from_coords(coords), s));
}

PyObject* create_Raster(const Either<Point, Rect>& region,
  const Bitmap& bmp,
  const Optional<Settings>& maybeSettings)
{
  Rect r = region.Visit(
    [&bmp](const Point& pt){
      return Rect(pt, floated(bmp.GetSize()));
    },
    [](const Rect& r){
      return r;
    });


  const auto tri = tri_from_rect(r);
  const auto s = maybeSettings.Or(default_raster_settings());
  return create_Shape(new ObjRaster(tri_from_rect(r), bmp, s));
}

PyObject* create_Spline(const std::vector<coord>& coords,
  const Optional<Settings>& maybeSettings)
{
  if (coords.empty()){
    throw ValueError("No coordinates specified.");
  }
  if (coords.size() % 2 != 0){
    throw ValueError("Uneven number of coordinates.");
  }

  const auto s = maybeSettings.Or(default_spline_settings());
  const auto pts = points_from_coords(coords);
  return create_Shape(create_spline_object(pts, s));
}

PyObject* create_Text(const Either<Rect, Point>& region,
  const utf8_string& text,
  const Optional<Settings>& maybeSettings)
{
  Settings s = maybeSettings.Or(default_text_settings());
  Rect r = region.Visit(
    [](const Rect& r){
      return r;
    },
    [&s](const Point& pt){
      s.Set(ts_BoundedText, false);
      return Rect(pt,
        Size(100,100)); // Fixme: Just a dummy size. Should auto-size instead.
    });

  const auto tri = tri_from_rect(r);
  return create_Shape(new ObjText(tri, text, s));
}

} // namespace
