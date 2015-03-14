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

#include "geo/tri.hh"
#include "python/mapped-type.hh"
#include "python/py-include.hh"
#include "python/py-tri.hh"
#include "python/py-ugly-forward.hh"

namespace faint{

template<>
struct MappedType<Tri&>{
  using PYTHON_TYPE = triObject;

  static Tri& GetCppObject(triObject* self){
    return self->tri;
  }

  static bool Expired(triObject*){
    // The triObject owns the Tri - it is not a reference
    // that can break.
    return false;
  }

  static void ShowError(triObject*){
  }

  static utf8_string DefaultRepr(const triObject*){
    return "Tri";
  }

};

/* method: "p0()->(x,y)\n
Returns a triangle vertex." */
static Point tri_p0(Tri& self){
  return self.P0();
}

/* method: "p1()->(x,y)\n
Returns a triangle vertex." */
static Point tri_p1(Tri& self){
  return self.P1();
}

/* method: "p2()->(x,y)\n
Returns a triangle vertex." */
static Point tri_p2(Tri& self){
  return self.P2();
}

/* method: "p3()->(x,y)\n
Returns p0() mirrored over p1() and p2()." */
static Point tri_p3(Tri& self){
  return self.P3();
}

/* method: "width()\n
Returns the width of the Tri, specified as the distance from p0 to
p1" */
static coord tri_width(Tri& self){
  return self.Width();
}

/* method: "tri_height()\n
Returns the height of the Tri, specified as the distance from p0 to
p2." */
static coord tri_height(Tri& self){
  return self.Height();
}

/* method: "tri_rotate(radians, (x,y))\n
Rotate the tri the given radians around x,y." */
static void tri_rotate(Tri& self,
  const Angle& angle,
  const Optional<Point>& pivot)
{
  self = pivot.IsSet() ?
    rotated(self, angle, pivot.Get()) :
    rotated(self, angle, self.P3());
}

/* method: "translate(dx,dy)\n
Offsets the Tri by dx, dy aligned with the image." */
static void tri_translate(Tri& self, const Point& delta){
  self = translated(self, delta.x, delta.y);
}

/* method: "center()->x,y\n
Returns the center point of the tri, adjusted for rotation (the
intersection of p0p3 and p1p2)" */
static Point tri_center(Tri& self){
  return center_point(self);
}

/* method: "offset_aligned(dx,dy)\n
Offsets the tri by dx, dy aligned with its axes (dx is along p0p1 and
dy along p0p2" */
static void tri_offset_aligned(Tri& self, const Point& delta){
  self = offset_aligned(self, delta.x, delta.y);
}

static PyObject* tri_new(PyTypeObject *type, PyObject*, PyObject*){
  triObject* self;
  self = (triObject*)type->tp_alloc(type, 0);
  return (PyObject*)self;
}

static void tri_init(triObject& self,
  const Point& p0,
  const Point& p1,
  const Point& p2)
{
  self.tri = Tri(p0,p1,p2);
}

static utf8_string tri_repr(Tri& self){
  return no_sep("Tri",bracketed(comma_sep(
    bracketed(str(self.P0())),
    bracketed(str(self.P1())),
    bracketed(str(self.P2())))));
}

/* property: "angle\n
The angle between p0()->p1() and the horizon, expressed in radians." */
struct tri_angle{
  static coord Get(Tri& self){
    return self.GetAngle().Rad();
  }

  static void Set(Tri& self, const Angle& angle){
    self = rotated(rotated(self, -self.GetAngle(), self.P0()),
      angle, self.P0());
  }
};

/* property: "skew\n
Specifies the skewness of the Tri (the distance
between p0 and a line perpendicular to p0p1 through p2." */
struct tri_skew{
  static coord Get(Tri& self){
    return self.Skew();
  }

  static void Set(Tri& self, coord skew){
    self = skewed(skewed(self, -self.Skew()),
      skew);
  }
};

#include "generated/python/method-def/py-tri-methoddef.hh"

PyTypeObject TriType = {
  PyVarObject_HEAD_INIT(nullptr, 0)
  "Tri", // tp_name
  sizeof(triObject), //tp_basicsize
  0, // tp_itemsize
  nullptr, // tp_dealloc
  nullptr, // tp_print
  nullptr, // tp_getattr
  nullptr, // tp_setattr
  nullptr, // tp_compare
  REPR_FORWARDER(tri_repr), // tp_repr
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
  "A Tri defines the geometry of an object", // tp_doc
  nullptr, // tp_traverse */
  nullptr, // tp_clear */
  nullptr, // tp_richcompare */
  0, // tp_weaklistoffset */
  nullptr, // tp_iter */
  nullptr, // tp_iternext */
  tri_methods, // tp_methods */
  nullptr, // tp_members
  tri_getseters, // tp_getset */
  nullptr, // tp_base */
  nullptr, // tp_dict */
  nullptr, // tp_descr_get */
  nullptr, // tp_descr_set */
  0, // tp_dictoffset */
  INIT_FORWARDER(tri_init), // tp_init
  nullptr, // tp_alloc
  tri_new, // tp_new */
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

PyObject* pythoned(const Tri& tri){
  triObject* py_tri = (triObject*) TriType.tp_alloc(&TriType, 0);
  py_tri->tri = tri;
  return (PyObject*)py_tri;
}

} // namespace
