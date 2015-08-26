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

#include "geo/rect.hh"
#include "geo/tri.hh"
#include "objects/object.hh"
#include "objects/objrectangle.hh"
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

/* method: "get_tri()" */
static Tri Shape_get_tri(Object& self){
  return self.GetTri();
}

/* method: "get_tri()" */
static void Shape_set_tri(Object& self, const Tri& tri){
  return self.SetTri(tri);
}

// TODO: Remove, early test method
/* method: "get_type()" */
static utf8_string Shape_get_type(Object& self){
  return self.GetType();
}

/* method: "get_rect() -> (x,y,w,h)\n
Returns the bounding rectangle. " */
static Rect Shape_rect(Object& self){
  return bounding_rect(self.GetTri());
}

static void Shape_init(shapeObject& self){
  const Tri t(Point(0.0, 0.0),
    Point(10.0, 0.0),
    Point(0.0, 10.0));
  const auto s = default_rectangle_settings();
  self.obj = create_rectangle_object(t, s);
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

shapeObject* create_Shape(){
  return (shapeObject*)ShapeType.tp_alloc(&ShapeType, 0);
}

PyObject* create_Rect(const Rect& r, const Optional<Settings>& maybeSettings){
  auto shape = create_Shape();
  const auto s = maybeSettings.Visit([](const Settings& s){return s;},
    default_rectangle_settings);
  shape->obj = create_rectangle_object(tri_from_rect(r), s);
  return (PyObject*)shape;
}

} // namespace
