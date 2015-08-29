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

#include "python/py-include.hh"
#include "python/py-image-props.hh"
#include "python/mapped-type.hh"
#include "python/py-ugly-forward.hh"
#include "util/grid.hh"
#include "util/image-props.hh"

namespace faint{

template<>
struct MappedType<ImageProps&>{
  using PYTHON_TYPE = imagePropsObject;

  static ImageProps& GetCppObject(imagePropsObject* self){
    return *self->props;
  }

  static bool Expired(imagePropsObject* self){
    return !(self->alive);
  }

  static void ShowError(imagePropsObject*){
    PyErr_SetString(PyExc_ValueError,
      "Operation attempted on retired ImageProps.");
  }

  static utf8_string DefaultRepr(imagePropsObject*){
    return "Retired ImageProps";
  }
};


template<>
struct MappedType<imagePropsObject&>{
  using PYTHON_TYPE = imagePropsObject;
  static imagePropsObject& GetCppObject(imagePropsObject* self){
    return *self;
  }

  static bool Expired(imagePropsObject* self){
    return !(self->alive);
  }

  static void ShowError(imagePropsObject*){
    PyErr_SetString(PyExc_ValueError,
      "Operation attempted on expired ImageProps");
  }
};

/* method: "get_frame(i)->frame\nReturns the i:th frame" */
static PyObject* imageprops_get_frame(imagePropsObject& self, const Index& index){
  throw_if_outside(index, self.props->GetNumFrames());
  return create_FrameProps(self, index);
}

/* method: "set_error(s)\n
Sets an error state described by the string s for this ImageProps.
This will inhibit opening a new image tab." */
static void imageprops_set_error(ImageProps& self, const utf8_string& err){
  self.SetError(err);
}

/* method: "set_grid((x,y), dashed, enabled, spacing)\n
Specify the grid." */
static void frameprops_set_grid(ImageProps& self,
  const Point& anchor,
  bool dashed,
  bool enabled,
  int spacing)
{
  Grid g(enabled, spacing, default_grid_color(), anchor);
  g.SetDashed(dashed);
  self.SetGrid(g);
}

/* method: "add_warning(s)\n
Adds a warning note to indicate a non-fatal problem loading a file" */
static void imageprops_add_warning(ImageProps& self, const utf8_string& warn){
  self.AddWarning(warn);
}

/* method: "add_frame([width,height])->frame\n
Appends a frame with the (optionally-specified) width and height" */
static PyObject* imageprops_add_frame(imagePropsObject& self,
  const Optional<IntSize>& size)
{
  self.props->AddFrame(ImageInfo(size.Or(IntSize(640,480)),
    create_bitmap(false)));
  return create_FrameProps(self, self.props->GetNumFrames() - 1);
}

#include "generated/python/method-def/py-image-props-method-def.hh"

// Python standard methods follow...
static PyObject* imageprops_new(PyTypeObject* type, PyObject*, PyObject*){
  imagePropsObject* self;
  self = (imagePropsObject*)type->tp_alloc(type, 0);
  return (PyObject *)self;
}

static utf8_string imageprops_repr(ImageProps&){
  return "ImageProps";
}

static void imageprops_init(imagePropsObject& self){
  self.props = new ImageProps();

  // The image-props must manage its own memory when initialized
  // from Python (compare with pythoned(ImageProps&)
  self.owner = true;
  self.alive = true;
}

static void imageprops_dealloc(imagePropsObject* self){
  // Fixme: Verify that dealloc is where I should do this
  // (and not e.g. tp_free or tp_del)
  if (self->owner){
    self->alive = false;
    self->owner = false;
    delete self->props;
  }
  self->ob_base.ob_type->tp_free((PyObject*)self);
}

PyTypeObject ImagePropsType = {
  PyVarObject_HEAD_INIT(nullptr, 0)
  "ImageProps", //tp_name
  sizeof(imagePropsObject), // tp_basicsize
  0, // tp_itemsize
  (destructor)imageprops_dealloc, // tp_dealloc
  nullptr, // tp_print
  nullptr, // tp_getattr
  nullptr, // tp_setattr
  nullptr, // tp_compare
  REPR_FORWARDER(imageprops_repr), // tp_repr
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
  "Used for defining an image", // tp_doc
  nullptr, // tp_traverse
  nullptr, // tp_clear
  nullptr, // tp_richcompare
  0, // tp_weaklistoffset
  nullptr, // tp_iter
  nullptr, // tp_iternext
  imageprops_methods, // tp_methods
  nullptr, // tp_members
  nullptr, // tp_getset
  nullptr, // tp_base
  nullptr, // tp_dict
  nullptr, // tp_descr_get
  nullptr, // tp_descr_set
  0, // tp_dictoffset
  INIT_FORWARDER(imageprops_init), // tp_init
  nullptr, // tp_alloc
  imageprops_new, // tp_new
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

typed_scoped_ref<imagePropsObject> pythoned(ImageProps& props){
  imagePropsObject* py_props = (imagePropsObject*)
    ImagePropsType.tp_alloc(&ImagePropsType,0);
  py_props->props = &props;
  py_props->alive = true;

  // The ImageProps memory is owned by the caller.
  py_props->owner = false;
  return typed_scoped_ref<imagePropsObject>(py_props);
}

} // namespace
