// -*- coding: us-ascii-unix -*-
// Copyright 2016 Lukas Kemmer
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

#include "python/py-color.hh"
#include "python/mapped-type.hh"
#include "text/formatting.hh"
#include "text/utf8-string.hh"
#include "python/py-ugly-forward.hh"
#include "python/py-add-type-object.hh"

namespace faint{

extern PyTypeObject ColorType;

struct colorObject{
  PyObject_HEAD;
  Color color;
};

template<>
struct MappedType<Color&>{
  using PYTHON_TYPE = colorObject;

  static Color& GetCppObject(colorObject* self){
    return self->color;
  }

  static bool Expired(colorObject*){
    return false; // Color owned by Python
  }

  static void ShowError(colorObject*){
    // Can't expire
  }

  static utf8_string DefaultRepr(const colorObject*){
    return "Color"; // Can't happen
  }
};

static void Color_init(colorObject& self, PyObject* args){
  const auto len = PySequence_Length(args);
  if (len == 0){
    self.color = color_black;
  }
  else if (len == 3){
    int r, g, b;
    if (!PyArg_ParseTuple(args, "iii", &r, &g, &b)){
      throw PresetFunctionError();
    }
    if (!valid_color(r, g, b)){
      throw ValueError("Color argument out of range 0-255");
    }
    self.color = color_from_ints(r, g, b);
  }
  else if (len == 4){
    int r, g, b, a;
    if (!PyArg_ParseTuple(args, "iiii", &r, &g, &b, &a)){
      throw PresetFunctionError();
    }

    if (!valid_color(r, g, b, a)){
      throw ValueError("Color argument out of range 0-255");
    }
    self.color = color_from_ints(r, g, b, a);
  }
  else{
    throw TypeError(space_sep("Color takes 0, 3 or 4 arguments",
        bracketed(space_sep(str_int(len), "given"))));
  }
}

static utf8_string Color_repr(Color& self){
  return no_sep("Color", bracketed(str_rgba(self)));
}

static void Color_dealloc(colorObject* self){
  self->ob_base.ob_type->tp_free((PyObject*)self);
}

/* method: "__copy__()->color\n
Used by Python copy.copy"
name: "__copy__" */
static Color Color_copy(Color& c){
  return c;
}

PyObject* Color_richcompare(colorObject* self, PyObject* otherRaw, int op){
  if (PyObject_TypeCheck(otherRaw, &ColorType)){
    colorObject* other((colorObject*)otherRaw);
    const Color& lhs = self->color;
    const Color& rhs = other->color;
    return py_rich_compare(lhs, rhs, op);
  }
  else if (PySequence_Check(otherRaw)){
    int r, g, b, a;
    if (!PyArg_ParseTuple(otherRaw, "iiii", &r, &g, &b, &a)){
      PyErr_Clear();
      Py_RETURN_FALSE;
    }
    if (!valid_color(r,g,b,a)){
      Py_RETURN_FALSE;
    }
    const auto& lhs = self->color;
    const auto rhs = color_from_ints(r,g,b,a);
    return py_rich_compare(lhs, rhs, op);
  }
  else{
    PyErr_Clear();
    Py_RETURN_NOTIMPLEMENTED;
  }
}

static PyObject* Color_new(PyTypeObject* type, PyObject*, PyObject*){
  colorObject* self;
  self = (colorObject*)type->tp_alloc(type, 0);
  return (PyObject*)self;
}

using color_value_t = StaticBoundedInt<0, 255>;

/* property: "r (int)\n
Red (0-255)." */
struct py_r{
 static int Get(Color& self){
   return self.r;
 }

 static void Set(Color& self, const color_value_t& r){
   self.r = static_cast<faint::uchar>(r.GetValue());
 }
};

/* property: "g (int)\n
Green (0-255)." */
struct py_g{
 static int Get(Color& self){
   return self.g;
 }

 static void Set(Color& self, const color_value_t& g){
   self.g = static_cast<faint::uchar>(g.GetValue());
 }
};

/* property: "b (int)\n
Blue (0-255)." */
struct py_b{
 static int Get(Color& self){
   return self.b;
 }

 static void Set(Color& self, const color_value_t& b){
   self.b = static_cast<faint::uchar>(b.GetValue());
 }
};

/* property: "a (int)\n
Alpha (0-255)." */
struct py_a{
 static int Get(Color& self){
   return self.a;
 }

 static void Set(Color& self, const color_value_t& a){
   self.a = static_cast<faint::uchar>(a.GetValue());
 }
};

namespace {
// Mapping methods, for allowing indexing the color as well as slicing:
// c[0] is the same as c.r
// c[0:2] is the same as c.r, c.g

static Py_ssize_t Color_length(PyObject*){
  return 4;
}

static int get_item(const Color& c, Py_ssize_t i){
  if (i == 0){
    return c.r;
  }
  else if (i == 1){
    return c.g;
  }
  else if (i == 2){
    return c.b;
  }
  else if (i == 3){
    return c.a;
  }
  assert(false);
  return 0;
}

static PyObject* Color_slice(const Color& c, Py_ssize_t start, Py_ssize_t stop){
  Py_ssize_t len = stop - start;
  auto* l = PyTuple_New(len);
  for (Py_ssize_t i = 0; i != len; i++){
    PyTuple_SetItem(l, i, Py_BuildValue("i", get_item(c, i + start)));
  }
  return l;
}

static PyObject* Color_Subscript(PyObject* o, PyObject* key){
  if (PyIndex_Check(key)){
    Py_ssize_t i = PyNumber_AsSsize_t(key, PyExc_IndexError);
    if (i == -1 && PyErr_Occurred()){
      return nullptr;
    }
    if (0 <= i && i < 4){
      const auto* co = (colorObject*)o;
      return Py_BuildValue("i", get_item(co->color, i));
    }
    else{
      PyErr_SetString(PyExc_IndexError, "Color index out of range");
      return nullptr;
    }
  }
  else if (PySlice_Check(key)){
    const int len = 4;
    Py_ssize_t start, stop, step, slicelength;
    if (PySlice_GetIndicesEx(key,
        len, &start, &stop,
        &step, &slicelength) < 0)
    {
      return nullptr;
    }
    if (slicelength <= 0){
      return PyTuple_New(0);
    }
    else if (step == 1){
      const auto* co = (colorObject*)o;
      return Color_slice(co->color, start, stop);
    }
    else{
      PyErr_SetString(PyExc_TypeError, "Color does not support slice steps");
      return nullptr;
    }
  }
  PyErr_SetString(PyExc_TypeError, "Color indices must be integers");
  return nullptr;
}

PyMappingMethods ColorMappingMethods = {
  Color_length, // mp_length
  Color_Subscript, // mp_subscript
  nullptr, // mp_ass_subscript
};

} // namespace

#include "generated/python/method-def/py-color-method-def.hh"

PyTypeObject ColorType = {
  PyVarObject_HEAD_INIT(nullptr, 0)
  "Color", // tp_name
  sizeof(colorObject), // tp_basicsize
  0, // tp_itemsize
  (destructor)Color_dealloc, // tp_dealloc
  nullptr, // tp_print
  nullptr, // tp_getattr
  nullptr, // tp_setattr
  nullptr, // tp_compare
  REPR_FORWARDER(Color_repr), // tp_repr
  nullptr, // tp_as_number
  nullptr, // tp_as_sequence
  &ColorMappingMethods, // tp_as_mapping
  nullptr, // tp_hash
  nullptr, // tp_call
  nullptr, // tp_str
  nullptr, // tp_getattro
  nullptr, // tp_setattro
  nullptr, // tp_as_buffer
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags
  // tp_doc
  "RGBA color.",
  nullptr, // tp_traverse
  nullptr, // tp_clear
  (richcmpfunc)Color_richcompare, // tp_richcompare
  0, // tp_weaklistoffset
  nullptr, // tp_iter
  nullptr, // tp_iternext
  color_methods, // tp_methods
  nullptr, // tp_members
  color_getseters, // tp_getset
  nullptr, // tp_base
  nullptr, // tp_dict
  nullptr, // tp_descr_get
  nullptr, // tp_descr_set
  0, // tp_dictoffset
  INIT_FORWARDER(Color_init), // tp_init
  nullptr, // tp_alloc
  Color_new, // tp_new
  nullptr, // tp_free
  nullptr, // tp_is_gc
  nullptr, // tp_bases
  nullptr, // tp_mro
  nullptr, // tp_cache
  nullptr, // tp_subclasses
  nullptr, // tp_weaklist
  nullptr, // tp_del
  0, // tp_version_tag
  nullptr // tp_finalize
};

Color* as_Color(PyObject* obj){
  if (PyObject_TypeCheck(obj, &ColorType)){
    colorObject* py_color = (colorObject*)obj;
    return &(py_color->color);
  }
  return nullptr;
}

PyObject* pythoned(const Color& c){
  colorObject* pyColor =
    (colorObject*)ColorType.tp_alloc(&ColorType, 0);
  pyColor->color = c;
  return (PyObject*)pyColor;
}

void add_type_Color(PyObject* module){
  add_type_object(module, ColorType, "Color");
}

} // namespace
