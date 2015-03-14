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

#include "bitmap/gradient.hh"
#include "python/mapped-type.hh"
#include "python/py-include.hh"
#include "python/py-radial-gradient.hh"
#include "python/py-ugly-forward.hh"
#include "python/py-util.hh"

namespace faint{

template<>
struct MappedType<RadialGradient&>{
  using PYTHON_TYPE = radialGradientObject;

  static RadialGradient& GetCppObject(radialGradientObject* self){
    return *self->gradient;
  }

  static bool Expired(radialGradientObject*){
    return false;
  }

  static void ShowError(radialGradientObject*){
  }
};

PyObject* RadialGradient_richcompare(radialGradientObject* self,
  PyObject* otherRaw,
  int op)
{
  if (!PyObject_IsInstance(otherRaw, (PyObject*)&RadialGradientType)){
    Py_RETURN_NOTIMPLEMENTED;
  }
  radialGradientObject* other((radialGradientObject*)otherRaw);
  const RadialGradient& lhs(*self->gradient);
  const RadialGradient& rhs(*other->gradient);
  return py_rich_compare(lhs, rhs, op);
}

static bool coord_from_py_noerr(PyObject* obj, coord& c){
  if (!PyNumber_Check(obj)){
    return false;
  }
  PyObject* pythonFloat = PyNumber_Float(obj);
  c = PyFloat_AsDouble(pythonFloat);
  return true;
}

static bool point_from_sequence_noerr(PyObject* obj, Point& p){
  if (!PySequence_Check(obj)){
    return false;
  }
  if (PySequence_Length(obj) != 2){
    return false;
  }
  scoped_ref py_x(PySequence_GetItem(obj, 0));
  coord x = 0.0;
  if (!coord_from_py_noerr(py_x.get(), x)){
    return false;
  }
  scoped_ref py_y(PySequence_GetItem(obj, 1));
  coord y = 0.0;
  if (!coord_from_py_noerr(py_y.get(), y)){
    return false;
  }
  p.x = x;
  p.y = y;
  return true;
}

static int RadialGradient_init(radialGradientObject* self,
  PyObject* args,
  PyObject*)
{
  int numArgs = static_cast<int>(PySequence_Length(args)); // Fixme: Check cast or change type
  if (numArgs == 0){
    PyErr_SetString(PyExc_ValueError, "Atleast one color stop required");
    return init_fail;
  }

  // Possibly unwrapped sequence
  PyObject* unwrapped = nullptr;

  // The center point is an optional argument (although the first!)
  Point center(0,0);

  // If the first argument is an point, the color stop parsing should be offset
  int firstColorStop = 0;

  if (numArgs == 1){
    // Check if this is a single color stop
    PyObject* pyStop = PySequence_GetItem(args, 0);
    ColorStop stop;
    if (parse_color_stop(pyStop, stop)){
      py_xdecref(pyStop);
      self->gradient = new RadialGradient(Point(0,0), Radii(1.0,1.0), {stop});
      return init_ok;
    }
    else if (!PySequence_Check(pyStop)){
      PyErr_SetString(PyExc_ValueError, "Atleast one color stop required");
      py_xdecref(pyStop);
      return init_fail;
    }
    numArgs = static_cast<int>(PySequence_Length(pyStop)); // Fixme: Check cast or change type
    if (numArgs == 0){
      PyErr_SetString(PyExc_ValueError, "Atleast one color stop required");
      return init_fail;
    }
    args = pyStop;
    unwrapped = pyStop;
  }
  else if (numArgs > 1){
    // Try to parse the first argument as a center point
    scoped_ref firstArg(PySequence_GetItem(args,0));
    if (PySequence_Check(firstArg.get()) &&
      PySequence_Length(firstArg.get()) == 2)
    {
      if (point_from_sequence_noerr(firstArg.get(), center)){
        firstColorStop += 1;
      }
    }
  }

  std::vector<ColorStop> v;
  for (int i = firstColorStop; i != numArgs; i++){
    PyObject* pyStop = PySequence_GetItem(args, i);
    ColorStop stop;
    bool ok = parse_color_stop(pyStop, stop);
    py_xdecref(pyStop);
    if (!ok){
      py_xdecref(unwrapped);
      return init_fail;
    }
    v.push_back(stop);
  }

  self->gradient = new RadialGradient(center, Radii(1.0,1.0), v);
  py_xdecref(unwrapped);
  return init_ok;
}

static PyObject* RadialGradient_new(PyTypeObject* type, PyObject*, PyObject*){
  radialGradientObject* self = (radialGradientObject*)type->tp_alloc(type, 0);
  self->gradient = nullptr;
  return (PyObject*)self;
}

static PyObject* RadialGradient_repr(radialGradientObject* self){
  return Py_BuildValue("s", get_repr(*self->gradient).c_str());
}

static void RadialGradient_dealloc(radialGradientObject* self){
  delete self->gradient;
  self->gradient = nullptr;
  self->ob_base.ob_type->tp_free((PyObject*)self);
}

/* method: "add_stop(offset,(r,g,b[,a]))\n
Adds the specified color stop to the gradient" */
static void RadialGradient_add_stop(RadialGradient& self, const ColorStop& stop){
  self.Add(stop);
}

/* method: "get_center()->x,y\n
Returns the center point of the gradient" */
static Point RadialGradient_get_center(RadialGradient& self){
  return self.GetCenter();
}

/* method: "get_num_stops()->i\n
Returns the number of color stops in the gradient" */
static Index RadialGradient_get_num_stops(RadialGradient& self){
  return self.GetNumStops();
}

/* method: "get_object_aligned()->b\n
True if the gradient is aligned with the objects, False if aligned
with the image" */
static bool RadialGradient_get_object_aligned(RadialGradient& self){
  return self.GetObjectAligned();
}

/* method: "get_radii()->b\nReturns the radiuses of the gradient" */
static Radii RadialGradient_get_radii(RadialGradient& self){
  return self.GetRadii();
}

/* method: "get_stop(i)->(offset, (r,g,b[,a]))\n
Gets the color stop with the specified index" */
static ColorStop RadialGradient_get_stop(RadialGradient& self,
  const Index& index)
{
  if (self.GetNumStops() <= index){
    throw ValueError("Invalid color-stop index");
  }
  return self.GetStop(index);
}

/* method: "get_stops()->list\nReturns a list of color stops" */
static color_stops_t RadialGradient_get_stops(RadialGradient& self){
  return self.GetStops();
}

/* method: "set_center(x,y)\nSets the center point of the gradient" */
static void RadialGradient_set_center(RadialGradient& self,
  const Point& center)
{
  self.SetCenter(center);
}

/* method: "set_object_aligned(b)\nSets whether the gradient is aligned
with objects" */
static void RadialGradient_set_object_aligned(RadialGradient& self, bool aligned){
  self.SetObjectAligned(aligned);
}

/* method: "set_radii(rx, ry)\nSets the radiuses of the gradient" */
static void RadialGradient_set_radii(RadialGradient& self, const Radii& radii){
  self.SetRadii(radii);
}

// LinearGradient_methods is generated.
#include "generated/python/method-def/py-radial-gradient-methoddef.hh"

PyTypeObject RadialGradientType = {
  PyVarObject_HEAD_INIT(nullptr, 0)
  "RadialGradient", // tp_name
  sizeof(radialGradientObject), // tp_basicsize
  0, // tp_itemsize
  (destructor)RadialGradient_dealloc, // tp_dealloc
  nullptr, // tp_print
  nullptr, // tp_getattr
  nullptr, // tp_setattr
  nullptr, // reserved (formerly tp_compare)
  (reprfunc)RadialGradient_repr, // tp_repr
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
  "Faint Color Gradient", // tp_doc
  nullptr, // tp_traverse
  nullptr, // tp_clear
  (richcmpfunc)RadialGradient_richcompare, // tp_richcompare
  0, // tp_weaklistoffset
  nullptr, // tp_iter
  nullptr, // tp_iternext
  RadialGradient_methods, // tp_methods
  nullptr, // tp_members
  nullptr, // tp_getset
  nullptr, // tp_base
  nullptr, // tp_dict
  nullptr, // tp_descr_get
  nullptr, // tp_descr_set
  0, // tp_dictoffset
  (initproc)RadialGradient_init, // tp_init
  nullptr, // tp_alloc
  RadialGradient_new, // tp_new
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

} // namespace
