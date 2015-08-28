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

#include "bitmap/filter.hh"
#include "util/command-util.hh" // Fixme: Remove
#include "util/image.hh"
#include "util/image-util.hh"
#include "python/py-add-type-object.hh"
#include "python/py-image-props.hh"
#include "python/py-image.hh"
#include "python/py-common.hh"
#include "python/py-ugly-forward.hh"
#include "bitmap/aa-line.hh"
#include "bitmap/auto-crop.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/bitmap-exception.hh"
#include "bitmap/color.hh"
#include "bitmap/draw.hh"
#include "bitmap/filter.hh"
#include "bitmap/gaussian-blur.hh"
#include "bitmap/quantize.hh"

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
    // Can't expire
    return false;
  }

  static void ShowError(imageObject*){
    // Can't expire
  }
};

/* method: "get_pixel((x,y))->(r,g,b,a)
Returns the background color at x, y. Note: Ignores objects." */
static Color Image_get_pixel(Image& image, const IntPoint& pos){
  return image.GetBackground().Visit(
    [&pos](const Bitmap& bmp){
      throw_if_outside(pos, bmp);
      return get_color(bmp, pos);
    },
    [&pos](const ColorSpan& span){
      throw_if_outside(pos, span);
      return span.color;
    });
}

/* method: "get_size()" */
static IntSize Image_get_size(Image& self){
  return self.GetSize();
}

/* method: "num_objects()" */
static int Image_num_objects(Image& self){
  return self.GetNumObjects();
}

/* method: "set_pixel((x,y),(r,g,b,a))\n
Set the pixel at x,y to the specified color." */
static void Image_set_pixel(Image& self, const IntPoint& pt, const Color& c)
{
  if (!point_in_image(self, pt)){
    throw ValueError("Point outside image.");
  }

  self.GetBackground().Visit(
    [&](Bitmap& bmp){
      put_pixel(bmp, pt, c);
    },
    [&](ColorSpan){
      Bitmap& bmp = self.ConvertColorSpanToBitmap(); // Fixme: Might throw
      put_pixel(bmp, pt, c);
    });
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

using common_type = Image&;

// Specializations since Image doesn't support commands
template<>
void Common_aa_line<Image&>(Image&, const IntLineSegment&,
  const ColRGB&)
{}

template<>
bool Common_auto_crop(Image&){
  return false;
}

template<>
void Common_blit(Image&, const IntPoint&, const Bitmap&){
}

template<>
void Common_boundary_fill(Image&, const IntPoint&, const Paint&, const Color&)
{}

template<>
void Common_clear(Image&, const Paint&){}

template<>
void Common_color_balance(Image&,
  const color_range_t&,
  const color_range_t&,
  const color_range_t&)
{}

template<>
int Common_color_count(const Image& image){
  return image.GetBackground().Visit(
    [](const Bitmap& bmp){
      return count_colors(bmp);
    },
    [](const ColorSpan&){
      return 1;
    });
}

template<>
void Common_desaturate(Image& image){
  image.GetBackground().Visit(
    [](Bitmap& bmp){
      desaturate_simple(bmp);
    },
    [](ColorSpan& span){
      span.color = desaturated_simple(span.color);
    });
}

template<>
void Common_desaturate_weighted(Image&){
}

template<>
void Common_erase_but_color(Image&, const Color&, const Optional<Paint>&)
{}

template<>
void Common_flip_horizontally(Image&){
}

template<>
void Common_flip_vertically(Image&){
}

template<>
void Common_fill(Image&, const IntPoint&, const Paint&){
}

template<>
void Common_gaussian_blur(Image&, coord){}

template<>
void Common_invert(Image&){}

template<>
void Common_apply_paste(Image&, const IntPoint&, const Bitmap&){}

template<>
void Common_pixelize(Image&, const pixelize_range_t&){
}

template<>
void Common_quantize(Image&){
}

template<>
void Common_replace_alpha(Image&, const ColRGB&){
}

template<>
void Common_replace_color(Image&, const Color&, const Paint&){}

template<>
void Common_rotate(Image&, const Angle&, const Optional<Paint>&){}

template<>
void Common_sepia(Image&, int){}

template<>
void Common_set_alpha(Image&, const color_value_t&){}

template<>
void Common_set_threshold(Image&, const threshold_range_t&,
  const Optional<Paint>&, const Optional<Paint>&)
{}

#define COMMONFWD(bundle)FORWARDER(bundle::Func<Image&>, bundle::ArgType(), bundle::Name(), bundle::Doc())

/* extra_include: "generated/python/method-def/py-common-method-def.hh" */
#include "generated/python/method-def/py-image-method-def.hh"

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
