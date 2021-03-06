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

#include "bitmap/aa-line.hh"
#include "bitmap/auto-crop.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/bitmap-exception.hh"
#include "bitmap/color.hh"
#include "bitmap/draw.hh"
#include "bitmap/filter.hh"
#include "bitmap/gaussian-blur.hh"
#include "bitmap/quantize.hh"
#include "geo/axis.hh"
#include "text/text-expression-context.hh"
#include "util/at-most.hh"
#include "rendering/faint-dc.hh"
#include "util/optional.hh"
#include "objects/object.hh"
#include "python/py-common.hh"
#include "python/mapped-type.hh"
#include "python/py-include.hh"
#include "python/py-add-type-object.hh"
#include "python/py-bitmap.hh"
#include "python/py-tri.hh"
#include "python/py-util.hh"
#include "python/py-ugly-forward.hh"

namespace faint{

template<typename FUNC>
auto bmp_exception_to_py(FUNC&& func){
  try{
    return func();
  }
  catch (const BitmapOutOfMemory&){
    throw MemoryError("Failed allocating memory for Bitmap");
  }
  catch (const BitmapStrideError&){
    throw MemoryError("Failed initializing bitmap stride (width too large?)");
  }
  catch (const BitmapException& e){
    throw MemoryError(space_sep("Bitmap error:", e.what()));
  }
}

template<>
struct MappedType<Bitmap&>{
  using PYTHON_TYPE = bitmapObject;

  static Bitmap& GetCppObject(bitmapObject* self){
    return self->bmp;
  }

  static bool Expired(bitmapObject* self){
    return !bitmap_ok(self->bmp);
  }

  static void ShowError(bitmapObject*){
    PyErr_SetString(PyExc_ValueError, "Operation attempted on bad bitmap.");
  }

  static utf8_string DefaultRepr(const bitmapObject*){
    return "Invalid Bitmap";
  }
};

static void Bitmap_init(bitmapObject& self,
  const IntSize& size,
  const Optional<Paint>& bg)
{
  bmp_exception_to_py([&](){
    if (size.w <= 0 || size.h <= 0){
      throw ValueError("Negative size");
    }
    self.bmp = Bitmap(size, bg.Or(Paint(color_white)));
  });
}

static utf8_string Bitmap_repr(Bitmap&){
  return "Bitmap";
}

static void Bitmap_dealloc(bitmapObject* self){
  self->ob_base.ob_type->tp_free((PyObject*)self);
}

/* method: "__copy__()->bmp\n
Used by Python copy.copy"
name: "__copy__" */
static Bitmap Bitmap_copy(Bitmap&);

PyObject* Bitmap_richcompare(bitmapObject* self, PyObject* otherRaw, int op){
  if (!PyObject_IsInstance(otherRaw, (PyObject*)&BitmapType)){
    Py_RETURN_NOTIMPLEMENTED;
  }
  if (op != Py_EQ){
    Py_RETURN_NOTIMPLEMENTED;
  }

  auto other((bitmapObject*)otherRaw);
  const Bitmap& lhs(self->bmp);
  const Bitmap& rhs(other->bmp);
  if (lhs == rhs){
    Py_RETURN_TRUE;
  }
  Py_RETURN_FALSE;
}

/* method: "copy()->bmp\n
Returns a copy of the bitmap."
name: "copy" */
static Bitmap Bitmap_copy(Bitmap& self){
  return self;
}

/* method: "draw_objects(objects)\n
Draw the objects onto this bitmap." */
static void Bitmap_draw_objects(Bitmap& self, const objects_t& objects){
  class NoCtx : public ExpressionContext{
    Optional<Calibration> GetCalibration() const override{
      return {};
    }

    const Object* GetObject(const utf8_string&) const override{
      return nullptr;
    }
  };

  FaintDC dc(self);
  NoCtx ctx;
  for (Object* obj : objects){
    obj->Draw(dc, ctx);
  }
}

/* method: "get_raw_rgb_string()->s\n
Returns the bitmap as a bytes object with binary rgb values." */
static std::string Bitmap_get_raw_rgb_string(Bitmap& self){
  std::string str;
  str.reserve(to_size_t(area(self.GetSize())*3));
  for (int y = 0; y != self.m_h; y++){
    for (int x = 0; x != self.m_w; x++){
      Color c(get_color_raw(self, x, y));
      str += static_cast<char>(c.r);
      str += static_cast<char>(c.g);
      str += static_cast<char>(c.b);
    }
  }
  return str;
}

/* method: "subbitmap((x,y,w,h))->Bitmap\n
Returns the bitmap inside the specified rectangle." */
static Bitmap Bitmap_subbitmap(Bitmap& self, const IntRect& r){
  if (!fully_inside(r, self)){
    throw ValueError("Rectangle extends outside bitmap.");
  }
  if (empty(r)){
    throw ValueError("Empty rectangle.");
  }
  return subbitmap(self, r);
}

/* method: "get_size()->w,h\n
Returns the width and height of the bitmap." */
static IntSize Bitmap_get_size(Bitmap& self){
  return self.GetSize();
}

/* method: "set_pixel((x,y),(r,g,b[,a]))\n
Set the pixel at (x,y) to the specified color." */
static void Bitmap_set_pixel(Bitmap& self, const IntPoint& pos, const Color& c){
  if (invalid_pixel_pos(pos, self)){
    throw PresetFunctionError();
  }
  put_pixel(self, pos, c);
}

/* method: "get_pixel(x,y) -> (r,g,b,a)\n
Get the color at (x,y) as an RGBA-tuple." */
static Color Bitmap_get_pixel(Bitmap& self, const IntPoint& pos){
  if (invalid_pixel_pos(pos, self)){
    throw PresetFunctionError();
  }
  return get_color(self, pos);
}

/* method: "line(x0,y0,x1,y1)\n
Draw a line from x0, y0 to x1, y1" */
static void Bitmap_line(Bitmap& self, const IntLineSegment& line, const Color& c){
  draw_line(self, line, solid_1px(c));
}

static PyObject* Bitmap_new(PyTypeObject* type, PyObject*, PyObject*){
  bitmapObject* self;
  self = (bitmapObject*)type->tp_alloc(type, 0);
  return (PyObject*)self;
}

using common_type = Bitmap&;

// Specializations since Bitmap doesn't support commands
template<>
void Common_aa_line<Bitmap&>(Bitmap& bmp, const IntLineSegment& line,
  const ColRGB& color)
{
  draw_line_aa_Wu(bmp, line, color);
}

template<>
bool Common_auto_crop(Bitmap& bmp){
  return get_auto_crop_rectangles(bmp).Visit(
  [](){
    // Do nothing if not auto-croppable
    return false;
  },
  [&bmp](const IntRect& r){
    bmp = subbitmap(bmp, r);
    return true;
  },
  [&bmp](const IntRect& r0, const IntRect&){
    bmp = subbitmap(bmp, r0);
    return true;
  });
}

template<>
void Common_blit(Bitmap& dst, const IntPoint& topLeft, const Bitmap& src){
  blit(offsat(src, topLeft), onto(dst));
}

template<>
void Common_boundary_fill(Bitmap& bmp, const IntPoint& pos, const Paint& fill,
  const Color& boundary)
{
  boundary_fill(bmp, pos, fill, boundary);
}

template<>
void Common_clear(Bitmap& bmp, const Paint& paint){
  clear(bmp, paint);
}

template<>
void Common_color_balance(Bitmap& bmp, const color_range_t& r,
  const color_range_t& g,
  const color_range_t& b)
{
  color_balance(bmp, r, g, b);
}

template<>
int Common_color_count(Bitmap& bmp){
  return count_colors(bmp);
}

template<>
void Common_desaturate(Bitmap& bmp){
  desaturate_simple(bmp);
}

template<>
void Common_desaturate_weighted(Bitmap& bmp){
  desaturate_weighted(bmp);
}

template<>
void Common_erase_but_color(Bitmap& bmp, const Color& keep,
  const Optional<Paint>& eraser)
{
  if (eraser.NotSet()){
    throw ValueError("Erase color not specified.");
  }
  if (keep == eraser.Get()){
    throw ValueError("Same erase color as the kept color");
  }
  erase_but(bmp, keep, eraser.Get());
}

template<>
void Common_flip_horizontally(Bitmap& bmp){
  flip(bmp, along(Axis::HORIZONTAL));
}

template<>
void Common_flip_vertically(Bitmap& bmp){
  flip(bmp, along(Axis::VERTICAL));
}

template<>
void Common_fill(Bitmap& bmp, const IntPoint& pos, const Paint& paint){
  if (!point_in_bitmap(bmp, pos)){
    throw ValueError("Fill origin outside Bitmap");
  }
  flood_fill(bmp, pos, paint);
}

template<>
void Common_gaussian_blur(Bitmap& bmp, coord sigma){
  gaussian_blur_fast(bmp, sigma);
}

template<>
void Common_invert(Bitmap& bmp){
  invert(bmp);
}

template<>
void Common_apply_paste(Bitmap& dst, const IntPoint& pos, const Bitmap& src){
  blit(offsat(src, pos), onto(dst));
}

template<>
void Common_pixelize(Bitmap& bmp, const pixelize_range_t& width){
  pixelize(bmp, width);
}

template<>
void Common_quantize(Bitmap& bmp){
  quantize(bmp, Dithering::ON);
}

template<>
void Common_replace_alpha(Bitmap& bmp, const ColRGB& color){
  blend_alpha(bmp, color);
}

template<>
void Common_replace_color(Bitmap& bmp, const Color& old,
  const Paint& replacement)
{
  replace_color(bmp, Old(old), replacement);
}

template<>
void Common_rotate(Bitmap& bmp, const Angle& angle, const Optional<Paint>& bg){
  if (bg.NotSet()){
    throw ValueError("No background specified!");
  }
  bmp = rotate_bilinear(bmp, angle, bg.Get());
}

template<>
void Common_sepia(Bitmap& bmp, int intensity){
  sepia(bmp, intensity);
}

template<>
void Common_set_alpha(Bitmap& bmp, const color_value_t& alpha){
  set_alpha(bmp, static_cast<uchar>(alpha.GetValue()));
}

template<>
void Common_set_threshold(Bitmap& bmp, const std::pair<double, double>& range,
  const Optional<Paint>& in, const Optional<Paint>& out)
{
  if (in.NotSet()){
    throw ValueError("No inside fill specified");
  }
  if (out.NotSet()){
    throw ValueError("No outside fill specified");
  }
  const auto lower = constrained(Min(0.0), range.first, Max(1.0));
  const auto upper = constrained(Min(0.0), range.second, Max(1.0));
  const auto r = fractional_bounded_interval<threshold_range_t>(lower, upper);
  threshold(bmp, r, in.Get(), out.Get());
}

#define COMMONFWD(bundle)FORWARDER(bundle::Func<Bitmap&>, bundle::ArgType(), bundle::Name(), bundle::Doc())

/* extra_include: "generated/python/method-def/py-common-method-def.hh" */

/* property: "Bitmap size" */
struct bitmap_size{
  static IntSize Get(Bitmap& self){
    return self.GetSize();
  }

  static void Set(Bitmap& self, const IntSize& size){
    bmp_exception_to_py(
      [&](){
        Bitmap temp(size, color_white);
        blit(at_top_left(self), onto(temp));
        self = temp;
      });
  }
};


#include "generated/python/method-def/py-bitmap-method-def.hh"

PyTypeObject BitmapType = {
  PyVarObject_HEAD_INIT(nullptr, 0)
  "Bitmap", // tp_name
  sizeof(bitmapObject), // tp_basicsize
  0, // tp_itemsize
  (destructor)Bitmap_dealloc, // tp_dealloc
  0, // tp_vectorcall_offset
  nullptr, // tp_getattr
  nullptr, // tp_setattr
  nullptr, // tp_compare
  REPR_FORWARDER(Bitmap_repr), // tp_repr
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
  // tp_doc
  "Bitmap for RGBA pixel data.",
  nullptr, // tp_traverse
  nullptr, // tp_clear
  (richcmpfunc)Bitmap_richcompare, // tp_richcompare
  0, // tp_weaklistoffset
  nullptr, // tp_iter
  nullptr, // tp_iternext
  bitmap_methods, // tp_methods
  nullptr, // tp_members
  bitmap_getseters, // tp_getset
  nullptr, // tp_base
  nullptr, // tp_dict
  nullptr, // tp_descr_get
  nullptr, // tp_descr_set
  0, // tp_dictoffset
  INIT_FORWARDER(Bitmap_init), // tp_init
  nullptr, // tp_alloc
  Bitmap_new, // tp_new
  nullptr, // tp_free
  nullptr, // tp_is_gc
  nullptr, // tp_bases
  nullptr, // tp_mro
  nullptr, // tp_cache
  nullptr, // tp_subclasses
  nullptr, // tp_weaklist
  nullptr, // tp_del
  0, // tp_version_tag
  nullptr,  // tp_finalize
  nullptr, // tp_vectorcall
  nullptr // tp_print (deprecated)
};

void add_type_Bitmap(PyObject* module){
  add_type_object(module, BitmapType, "Bitmap");
}

} // namespace
