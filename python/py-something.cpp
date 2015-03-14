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

#include <vector>
#include <string>
#include "app/canvas.hh"
#include "app/get-app-context.hh"
#include "bitmap/bitmap.hh"
#include "commands/change-setting-cmd.hh"
#include "commands/obj-function-cmd.hh"
#include "commands/set-object-name-cmd.hh"
#include "commands/text-entry-cmd.hh"
#include "commands/tri-cmd.hh"
#include "geo/arc.hh" // For AngleSpan
#include "geo/int-rect.hh"
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
#include "python/py-canvas.hh"
#include "python/py-settings.hh"
#include "python/py-something.hh"
#include "python/py-tri.hh"
#include "python/py-ugly-forward.hh"
#include "python/py-util.hh" // For methods used by py-smth-properties.
#include "python/py-interface.hh"
#include "util/command-util.hh"
#include "util/image.hh"
#include "util/object-util.hh"
#include "util/setting-util.hh"
#include "util-wx/encode-bitmap.hh"
#include "util-wx/font.hh"
#include "python/py-something-properties.hh"
#include "generated/python/settings/py-smth-setting-properties.hh"

namespace faint{

template<class T>
struct MappedType<const BoundObject<T>&>{
  using PYTHON_TYPE = smthObject;

  static BoundObject<Object> GetCppObject(smthObject* self){
    return BoundObject<Object>(self->canvas, self->obj, self->frameId);
  }

  static bool Expired(smthObject* self){
    if (canvas_ok(self->canvasId) &&
      self->canvas->Has(self->frameId) &&
      self->canvas->GetFrame(self->frameId).Has(self->objectId)){
      return false;
    }
    return true;
  }

  static void ShowError(smthObject*){
    PyErr_SetString(PyExc_ValueError, "That object is removed.");
  }
};

PyObject* pythoned(Object* faintObj, Canvas* canvas, const FrameId& frameId){
  smthObject* smth = (smthObject*)SmthType.tp_alloc(&SmthType, 0);
  smth->obj = faintObj;
  smth->objectId = faintObj->GetId();
  smth->canvas = canvas;
  smth->canvasId = canvas->GetId();
  smth->frameId = frameId;
  return (PyObject*)smth;
}

// Helper for Smth_get_points
static std::vector<coord> flat_point_list(const std::vector<Point>& points){
  std::vector<coord> coords;
  coords.reserve(points.size() * 2);
  for (const Point& pt : points){
    coords.push_back(pt.x);
    coords.push_back(pt.y);
  }
  return coords;
}

static PyObject* Smth_richcompare(smthObject* self, PyObject* otherRaw, int op){
  if (!PyObject_IsInstance(otherRaw, (PyObject*)&SmthType)){
    Py_RETURN_NOTIMPLEMENTED;
  }
  smthObject* other((smthObject*)otherRaw);
  return py_rich_compare(self->objectId, other->objectId, op);
}

/* method: "become_path()->Path\n
Replaces this object with a corresponding Path object." */
static BoundObject<Object> Smth_become_path(const BoundObject<Object>& self){
  Object* path = clone_as_path(self.obj,
    self.canvas->GetFrame(self.frameId).GetExpressionContext());
  python_run_command(self, get_replace_object_command(Old(self.obj), path,
      self.canvas->GetFrame(self.frameId), select_added(false)));
  return BoundObject<Object>(self.canvas, path, self.frameId);
}

/* method: "crop()->p\n
Auto-crops a Raster or Text object." */
static bool Smth_crop(const BoundObject<Object>& self){
  if (ObjRaster* rasterObj = dynamic_cast<ObjRaster*>(self.obj)){
    if (Command* cmd = crop_raster_object_command(rasterObj)){
      python_run_command(self, cmd);
      return true;
    }
    return false;
  }
  else if (ObjText* textObj = dynamic_cast<ObjText*>(self.obj)){
    if (Command* cmd = crop_text_region_command(textObj)){
      python_run_command(self, cmd);
      return true;
    }
    return false;
  }
  else{
    throw TypeError(space_sep(self.obj->GetType(), "is not croppable."));
  }
}

/* method: "get_obj(i)->Object\n
Returns the sub-object specified by the passed in integer. Only
supported by groups." */
static auto Smth_get_obj(const BoundObject<Object>& self, int index)
{
  if (!is_composite(self.obj)){
    throw ValueError("This object does not support sub-objects");
  }

  if (index < 0 || self.obj->GetObjectCount() <= index){
    throw ValueError("Invalid object index");
  }
  return BoundObject<Object>(self.canvas,
    self.obj->GetObject(index), self.frameId);
}

/* method: "get_type()->s\nReturns the type of the object, as a string." */
static utf8_string Smth_get_type(const BoundObject<Object>& self){
  return self.obj->GetType();
}

/* method: "get_points()->(x0,y0,x1,y1,...)\nReturns a list of
vertices for Polygons, Splines and Paths. Returns the points in the
Tri for other objects." */
static std::vector<coord> Smth_get_points(const BoundObject<Object>& self){
  if (is_polygon(self.obj)){
    return flat_point_list(get_polygon_vertices(self.obj));
  }
  else if (is_spline(self.obj)) {
    return flat_point_list(get_spline_points(self.obj));
  }
  else if (is_line(self.obj)){
    return flat_point_list(self.obj->GetMovablePoints());
  }
  else {
    Tri t = self.obj->GetTri();
    return flat_point_list({t.P0(), t.P1(), t.P3(), t.P2()});
  }
}

/* method: "get_skew()->f\nReturns the skewness of the object." */
static coord Smth_get_skew(const BoundObject<Object>& self){
  Tri t = self.obj->GetTri();
  return t.Skew();
}

/* method: "get_settings()->Settings\n
Returns a copy of this object's settings." */
static Settings Smth_get_settings(const BoundObject<Object>& self){
  return self.obj->GetSettings();
}

/* method: "get_bitmap()->bmp\n
Returns a copy of the bitmap from a Raster object." */
static const Bitmap& Smth_get_bitmap(const BoundObject<Object>& self){
  ObjRaster* rasterObj = dynamic_cast<ObjRaster*>(self.obj);
  if (rasterObj == nullptr){
    throw ValueError("That object does not support fonts");
  }
  return rasterObj->GetBitmap();
}

/* method: "get_text_evaluated()->s\n
Returns the text from Text-object" */
static utf8_string Smth_get_text_evaluated(const BoundObject<Object>& self){
  ObjText* text = dynamic_cast<ObjText*>(self.obj);
  if (!text){
    throw ValueError(space_sep(self.obj->GetType(), "does not support text."));
  }
  const auto& image = self.canvas->GetFrame(self.frameId);
  auto& ctx = image.GetExpressionContext();
  return text->GetEvaluatedString(ctx);
}

/* method: "get_text_lines()->(s,...)\n
Returns the evaluated text from a Text-object split into lines. Takes
the bounding rectangle in consideration, so that the lines are split in
the same way as they appear in Faint." */
static text_lines_t Smth_get_text_lines(const BoundObject<Object>& self){
  ObjText* text = dynamic_cast<ObjText*>(self.obj);
  if (!text){
    throw TypeError(space_sep(self.obj->GetType(),
      "does not support text."));
  }

  const auto& image = self.canvas->GetFrame(self.frameId);
  auto& ctx = image.GetExpressionContext();
  return split_evaluated(ctx, text);
}

/* method: "get_text_baseline()->f\nReturns the offset from the top
of a row to the baseline. Relevant only for text objects" */
static coord Smth_get_text_baseline(const BoundObject<Object>& self){
  ObjText* txt = dynamic_cast<ObjText*>(self.obj);
  if (txt == nullptr){
    throw ValueError("This object does not have text attributes");
  }
  return txt->BaselineOffset();
}

/* method: "get_text_height(Returns the text height for this object.
Relevant only for text objects." */
static coord Smth_get_text_height(const BoundObject<Object>& self){
  ObjText* txt = dynamic_cast<ObjText*>(self.obj);
  if (txt == nullptr){
    throw ValueError("This object does not have text attributes");
  }
  return txt->RowHeight();
}

/* method: "num_objs()->i\n
Returns the number of sub-objects. Only useful for Group objects." */
static int Smth_num_objs(const BoundObject<Object>& self){
  if (!is_composite(self.obj)){
    throw ValueError("That object does not support sub-objects");
  }
  return self.obj->GetObjectCount();
}

/* method: "rect()->(x,y,w,h)\nReturns the bounding rectangle." */
static Rect Smth_rect(const BoundObject<Object>& self){
  return bounding_rect(self.obj->GetTri());
}

/* method: "rotate(a[,(x0,y0)])\n
Rotate this object a-radians clock-wise around its center or the
optionally specified pivot." */
static void Smth_rotate(const BoundObject<Object>& self,
  const Angle& angle,
  const Optional<Point>& origin)
{
  Tri oldTri(self.obj->GetTri());
  python_run_command(self,
    get_rotate_command(self.obj, angle,
      origin.Or(center_point(oldTri))));
}

/* method: "set_angles(a0,a1)\n
Sets the interior angles, specified in radians, for an elliptic arc" */
static void Smth_set_angles(const BoundObject<Object>& self,
  const AngleSpan& span)
{
  if (!is_ellipse(self.obj)){
    throw ValueError("Only supported for ellipses.");
  }

  using SetSpanCmd = ObjFunctionCommand<Object, AngleSpan, set_angle_span>;
  python_run_command(self,
    new SetSpanCmd(self.obj, "Change Arc Span",
      New(span), Old(get_angle_span(self.obj).Get())));
}

/* method: "set_text(s)\nSet the text of Text-objects" */
static void Smth_set_text(const BoundObject<Object>& self,
  const utf8_string& str)
{
  ObjText* text = dynamic_cast<ObjText*>(self.obj);
  if (!text){
    throw ValueError(space_sep(self.obj->GetType(), "does not support text."));
  }

  python_run_command(self,
    text_entry_command(text, New(str), Old(text->GetRawString())));
}

/* method: "get_text_raw(s)\nGet the unevaluated Text from Text objects." */
static utf8_string Smth_get_text_raw(const BoundObject<Object>& self){
  ObjText* text = dynamic_cast<ObjText*>(self.obj);
  if (!text){
    throw ValueError(space_sep(self.obj->GetType(), "does not support text."));
  }
  return text->GetTextBuffer().get();
}

/* method: "skew(f)\nSkew the object f-steps. See Tri for the skew definition." */
static void Smth_skew(const BoundObject<Object>& self, coord& skew){
  Object* obj = self.obj;
  Tri oldTri(obj->GetTri());
  Tri newTri(skewed(oldTri, skew));

  python_run_command(self,
    new TriCommand(obj, New(newTri), Old(oldTri)));
}

/* method: "update_settings(s)\nUpdate this object's settings with
those from the passed in Settings object.\nAny settings not relevant
for this object will be ignored." */
static void Smth_update_settings(const BoundObject<Object>& self,
  const Settings& s)
{
  python_run_command(self, change_settings_command(self.obj, New(s),
      Old(self.obj->GetSettings())));
}

static void Smth_init(smthObject& self){
  self.obj = nullptr;
  throw TypeError("Something objects can not be created manually. Use the "
    "designated object creation functions (e.g. Ellipse) instead.");
}

static PyObject* Smth_new(PyTypeObject* type, PyObject* , PyObject*){
  smthObject *self;
  self = (smthObject*)type->tp_alloc(type, 0);
  return (PyObject *)self;
}

static PyObject* Smth_repr(smthObject* self){
  if (!get_app_context().Exists(self->canvasId)){
    return Py_BuildValue("s", "Orphaned object");
  }
  else if (!self->canvas->Has(self->frameId)){
    return Py_BuildValue("s", "Orphaned object");
  }

  const Image& image = self->canvas->GetFrame(self->frameId);
  if (!image.Has(self->objectId)){
    return Py_BuildValue("s", "Removed object");
  }
  else {
    utf8_string str(self->obj->GetType());
    return Py_BuildValue("s", str.c_str()); // Fixme: utf8 c_str bad idea
  }
}

#include "generated/python/method-def/py-something-methoddef.hh"

PyTypeObject SmthType = {
  PyVarObject_HEAD_INIT(nullptr,0)
  "Something", // tp_name
  sizeof(smthObject), // tp_basicsize
  0, // tp_itemsize
  nullptr, // tp_dealloc
  nullptr, // tp_print
  nullptr, // tp_getattr
  nullptr, // tp_setattr
  nullptr, // tp_compare
  (reprfunc)Smth_repr, // tp_repr
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
  "The type for the Python-representation of Faint objects", // tp_doc
  nullptr, // tp_traverse
  nullptr, // tp_clear
  (richcmpfunc)Smth_richcompare, // tp_richcompare
  0, // tp_weaklistoffset
  nullptr, // tp_iter
  nullptr, // tp_iternext
  smth_methods, // tp_methods
  nullptr, // tp_members
  smth_properties, // tp_getset
  nullptr, // tp_base
  nullptr, // tp_dict
  nullptr, // tp_descr_get
  nullptr, // tp_descr_set
  0, // tp_dictoffset
  INIT_FORWARDER(Smth_init), // tp_init
  nullptr, // tp_alloc
  Smth_new, // tp_new
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
