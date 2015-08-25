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

#include "util/image.hh"
#include "python/py-image.hh"
#include "python/py-image-props.hh"
#include "python/py-ugly-forward.hh"
#include "python/py-add-type-object.hh"

namespace faint{

extern PyTypeObject ImageType;

struct imageObject{
  PyObject_HEAD
  Image* image;
};

template<>
struct MappedType<Image&>{
  using PYTHON_TYPE = imageObject;

  static Image& GetCppObject(imageObject* self){
    return *self->image;
  }

  static bool Expired(imageObject*){
    return false;
  }

  static void ShowError(imageObject*){
  }
};

/* method: "num_objects()" */
static int Image_num_objects(Image& self){
  return self.GetNumObjects();
}

static void Image_init(imageObject& self, PyObject* args){
  if (PySequence_Length(args) != 1){
    throw TypeError("Image requires one argument (FrameProps)");
  }
  PyObject* obj = PySequence_GetItem(args, 0); // Fixme: Leak?
  auto* p = get_cpp_FrameProps(obj);
  if (p == nullptr){
    throw TypeError("Image requires an unexpired FrameProps");
  }
  self.image = new Image(std::move(*p));
}

static void Image_dealloc(imageObject* self){
  delete self->image;
  self->image = nullptr;
  // Fixme: Call some base-dealloc? (cw py-bitmap.cpp)
}

static PyObject* Image_new(PyTypeObject* type, PyObject*, PyObject*){
  imageObject* self;
  self = (imageObject*)type->tp_alloc(type, 0);
  return (PyObject*)self;
}

#include "generated/python/method-def/py-image-methoddef.hh"

PyTypeObject ImageType = {
  PyVarObject_HEAD_INIT(nullptr, 0)
  "Image", // tp_name
  sizeof(imageObject), // tp_basicsize
  0, // tp_itemsize
  (destructor)Image_dealloc, // tp_dealloc
  nullptr, // tp_print
  nullptr, // tp_getattr
  nullptr, // tp_setattr
  nullptr, // tp_compare
  nullptr, // tp_repr
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
  "Image", // tp_doc // Fixme: Improve
  nullptr, // tp_traverse
  nullptr, // tp_clear
  nullptr, // tp_richcompare
  0, // tp_weaklistoffset
  nullptr, // tp_iter
  nullptr, // tp_iternext
  Image_methods, // tp_methods
  nullptr, // tp_members
  nullptr, // tp_getset
  nullptr, // tp_base
  nullptr, // tp_dict
  nullptr, // tp_descr_get
  nullptr, // tp_descr_set
  0, // tp_dictoffset
  INIT_FORWARDER(Image_init), // tp_init
  nullptr, // tp_alloc
  Image_new, // tp_new
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

void add_type_Image(PyObject* module){
  add_type_object(module, ImageType, "Image");
}

} // namespace
