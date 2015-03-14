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

#include <algorithm>
#include "bitmap/bitmap.hh"
#include "geo/points.hh"
#include "geo/rect.hh"
#include "objects/objcomposite.hh"
#include "objects/objellipse.hh"
#include "objects/objline.hh"
#include "objects/objpath.hh"
#include "objects/objpolygon.hh"
#include "objects/objraster.hh"
#include "objects/objrectangle.hh"
#include "objects/objspline.hh"
#include "objects/objtext.hh"
#include "python/py-include.hh"
#include "python/py-frame-props.hh"
#include "python/mapped-type.hh"
#include "python/py-ugly-forward.hh"
#include "python/py-util.hh"
#include "util/default-settings.hh"
#include "util/frame-props.hh"
#include "util/image-props.hh"
#include "util/setting-util.hh"

namespace faint{

void throw_if_missing(const BoundFrameProps& self, const Index& objectId){
  if (!self.frame.HasObject(objectId)){
    throw IndexError("Invalid object identifier");
  }
}

BoundFrameProps::BoundFrameProps(ImageProps& image, FrameProps& frame)
  : image(image),
    frame(frame)
{}

template<>
struct MappedType<const BoundFrameProps&>{
  using PYTHON_TYPE = framePropsObject;

  static BoundFrameProps GetCppObject(framePropsObject* self){
    return BoundFrameProps(*self->imageProps->props,
      self->imageProps->props->GetFrame(self->frame_index));
  }

  static bool Expired(framePropsObject* self){
    if (!self->imageProps->alive){
      return true;
    }
    return !(self->frame_index < self->imageProps->props->GetNumFrames());
  }

  static void ShowError(framePropsObject*){
    PyErr_SetString(PyExc_ValueError,
      "Operation attempted on expired FrameProps");
  }

  static utf8_string DefaultRepr(framePropsObject*){
    return "Retired FrameProps";
  }
};

/* method: "Group(id1, id2,...)->id\n
Groups the objects with the specified ids. Returns a new identifier for
the group." */
static Index frameprops_Group(const BoundFrameProps& self,
  const std::vector<Index>& ids)
{
  if (ids.empty()){
    throw ValueError("A Group must contain at least one object.");
  }

  objects_t objects;
  for (const Index& objectId : ids){
    throw_if_missing(self, objectId);
    if (!self.frame.IsTopLevel(objectId)){
      throw ValueError("Object is already in a group.");
    }
    objects.push_back(self.frame.GetObject(objectId));
  }
  objects_t::iterator uniqueEnd = std::unique(begin(objects), end(objects));
  if (uniqueEnd != end(objects)){
    throw ValueError("Duplicate object identifiers specified.");
  }

  // The composite owns the objects since they're not added
  // with an AddObjectCommand, but created during loading
  Object* composite = create_composite_object(objects, Ownership::OWNER);
  for (Object* obj : objects){
    self.frame.RemoveObject(obj);
  }

  Index groupId = self.frame.AddObject(composite);
  return groupId;
}

/* method: "Ellipse(x,y,w,h, settings) -> id\n
Creates an ellipse object and returns its id." */
static Index frameprops_Ellipse(const BoundFrameProps& self,
  const Rect& bounds,
  const Settings& s)
{
  return self.frame.AddObject(create_ellipse_object(tri_from_rect(bounds),
    updated(default_ellipse_settings(), s)));
}

/* method: "Line(x0,y0,x1,y1,settings) -> id\n
Creates a line object and returns its id." */
static Index frameprops_Line(const BoundFrameProps& self,
  const std::vector<coord>& coords,
  const Settings& s)
{
  if (coords.size() < 4){
    throw ValueError("At least four coordinates required for line"
      " (x0,y0,x1,y1).");
  }
  if (coords.size() % 2 != 0){
    throw ValueError("Number of coordinates must be an even number.");
  }
  return self.frame.AddObject(create_line_object(points_from_coords(coords),
    updated(default_line_settings(), s)));
}

/* method: "Path(s,settings) -> id\n
Creates a path object from the svg-path specification s, returns the
id of the path." */
static Index frameprops_Path(const BoundFrameProps& self,
  const utf8_string& path,
  const Settings& s)
{
  if (!is_ascii(path)){
    throw ValueError("Non-ascii-characters in path definition.");
  }
  std::vector<PathPt> points(parse_svg_path(path.str()));
  if (points.empty()){
    throw ValueError("Failed parsing path definition.");
  }
  if (points.front().IsNotMove()){
    throw ValueError("Paths must begin with a Move-entry.");
  }

  return self.frame.AddObject(create_path_object(Points(points),
    updated(default_path_settings(), s)));
}

/* method: "Polygon((x0,y0,x1,y1,...,xn,yn),settings)->id\n
Creates a Polygon from the sequence of coordinates and returns the
polygon id." */
static Index frameprops_Polygon(const BoundFrameProps& self,
  const std::vector<coord>& coords,
  const Settings& s)
{
  const size_t n = coords.size();
  if (n == 0){
    throw ValueError("No points specified.");
  }
  else if (n % 2 != 0){
    throw ValueError("Uneven number of coordinates.");
  }

  return self.frame.AddObject(
    create_polygon_object(points_from_coords(coords),
      updated(default_polygon_settings(), s)));
}

/* method: "Raster((x,y,w,h), Bitmap, settings)->id\n
Cretes a Raster object, scaled to the specified rectangle, and returns
its id.\n" */
static Index frameprops_Raster(const BoundFrameProps& self,
  const Rect& r,
  const Bitmap& bmp,
  const Settings& s)
{
  return self.frame.AddObject(new ObjRaster(tri_from_rect(r), bmp,
    updated(default_raster_settings(), s)));
}

/* method: "Rect((x,y,w,h),settings[,name])->id\n
Creates a Raster rectangle object and returns its id." */
static Index frameprops_Rect(const BoundFrameProps& self,
  const Rect& r,
  const Settings& s,
  const Optional<utf8_string>& name)
{
  Index index = self.frame.AddObject(create_rectangle_object(tri_from_rect(r),
    updated(default_rectangle_settings(), s)));
  if (name.IsSet()){
    self.frame.GetObject(index)->SetName(name);
  }
  return index;
}

/* method: "Spline((x0,y0,x1,y1,...,xn,yn),settings)->id\n
Creates a Spline object and returns its id." */
static Index frameprops_Spline(const BoundFrameProps& self,
  const std::vector<coord>& coords,
  const Settings& s)
{
  if (coords.empty()){
    throw ValueError("No coordinates specified.");
  }
  if (coords.size() % 2 != 0){
    throw ValueError("Uneven number of coordinates.");
  }
  return self.frame.AddObject(create_spline_object(points_from_coords(coords),
    updated(default_spline_settings(), s)));
}

/* method: "Text((x,y,w,h),text,settings)->id\n
Creates a Text object and returns its id." */
static Index frameprops_Text(const BoundFrameProps& self,
  const Rect& r,
  const utf8_string& text,
  const Settings& s)
{
  return self.frame.AddObject(new ObjText(tri_from_rect(r), text,
    updated(default_text_settings(), s)));
}

/* method: "set_calibration((x0,y0,x1,y1),length,unit)\n
Specifies the image calibration.." */
static void frameprops_set_calibration(const BoundFrameProps& self,
  const Calibration& c)
{
  self.frame.SetCalibration(c);
}

/* method: "set_obj_tri(id, tri)\n
Sets the Tri for the object specified by the id." */
static void frameprops_set_obj_tri(const BoundFrameProps& self,
  const Index& objectId,
  const Tri& tri)
{
  throw_if_missing(self, objectId);

  Object* object = self.frame.GetObject(objectId);
  object->SetTri(tri);
}

/* method: "set_obj_name(id, name)\n
Sets the name for the object specified by the id." */
static void frameprops_set_obj_name(const BoundFrameProps& self,
  const Index& objectId,
  const utf8_string& name)
{
  throw_if_missing(self, objectId);
  Object* object = self.frame.GetObject(objectId);
  object->SetName(option(name));
}

/* method: "get_obj_tri(id)->tri\n
Returns the Tri for the object specified by the id." */
static Tri frameprops_get_obj_tri(const BoundFrameProps& self,
  const Index& objectId)
{
  throw_if_missing(self, objectId);

  Object* object = self.frame.GetObject(objectId);
  return object->GetTri();
}

/* method: "get_obj_text_height(id)->height\n
Returns the height of the rows of the Text object specified by the id" */
static coord frameprops_get_obj_text_height(const BoundFrameProps& self,
  const Index& objectId)
{
  throw_if_missing(self, objectId);

  Object* object = self.frame.GetObject(objectId);
  ObjText* textObject = dynamic_cast<ObjText*>(object);
  if (textObject == nullptr){
    throw TypeError("Not a text object");
  }
  return textObject->BaselineOffset();
}

/* method: "get_size()->(w,h)\n
Returns the size of the frame." */
static IntSize frameprops_get_size(const BoundFrameProps& self){
  return self.frame.GetBackground().Visit(
    [](const Bitmap& bmp){
      return bmp.GetSize();
    },
    [](const ColorSpan& span){
      return span.size;
    });
}

/* method: "set_background(bitmap|color)\n
Specify the background for this frame as either a Bitmap or a
color." */
static void frameprops_set_background(const BoundFrameProps& self,
  const Either<Bitmap, ColorSpan>& bg)
{
  self.frame.SetBackground(bg);
}

/* method: "set_error(s)\n
Set an error state described by the string s for the ImageProps
containing this FrameProps. This will inhibit opening a new image
tab." */
static void frameprops_set_error(const BoundFrameProps& self,
  const utf8_string& err)
{
  self.image.SetError(err);
}

/* method: "add_warning(s)\n
Adds a warning note to indicate a non-fatal problem loading a
file." */
static void frameprops_add_warning(const BoundFrameProps& self,
  const utf8_string& warn)
{
  self.image.AddWarning(warn);
}

// Python standard methods follow...
static PyObject* frameprops_new(PyTypeObject* type, PyObject*, PyObject*){
  framePropsObject* self;
  self = (framePropsObject*)type->tp_alloc(type, 0);
  return (PyObject *)self;
}

static utf8_string frameprops_repr(const BoundFrameProps&){
  return "FrameProps";
}

static void frameprops_init(framePropsObject& self){
  self.imageProps = nullptr;
  throw TypeError("FrameProps can only be initialized via "
    "ImageProps.add_frame.");
}

static void frameprops_dealloc(framePropsObject* self){
  faint::py_xdecref((PyObject*)self->imageProps);
  self->imageProps = nullptr;
}

#include "generated/python/method-def/py-frame-props-methoddef.hh"

PyTypeObject FramePropsType = {
  PyVarObject_HEAD_INIT(nullptr, 0)
  "FrameProps", //tp_name
  sizeof(framePropsObject), // tp_basicsize
  0, // tp_itemsize
  (destructor)frameprops_dealloc, // tp_dealloc
  nullptr, // tp_print
  nullptr, // tp_getattr
  nullptr, // tp_setattr
  nullptr, // tp_compare
  REPR_FORWARDER(frameprops_repr), // tp_repr
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
  frame_props_methods, // tp_methods
  nullptr, // tp_members
  nullptr, // tp_getset
  nullptr, // tp_base
  nullptr, // tp_dict
  nullptr, // tp_descr_get
  nullptr, // tp_descr_set
  0, // tp_dictoffset
  INIT_FORWARDER(frameprops_init), // tp_init
  nullptr, // tp_alloc
  frameprops_new, // tp_new
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

} // namespace
