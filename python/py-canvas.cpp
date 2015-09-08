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

#include <sstream>
#include "app/app-context.hh"
#include "app/context-commands.hh"
#include "app/cut-and-paste.hh"
#include "app/frame.hh"
#include "bitmap/auto-crop.hh"
#include "commands/blit-bitmap-cmd.hh"
#include "commands/draw-object-cmd.hh"
#include "commands/flip-rotate-cmd.hh"
#include "commands/frame-cmd.hh"
#include "commands/group-objects-cmd.hh"
#include "commands/put-pixel-cmd.hh"
#include "commands/resize-cmd.hh"
#include "commands/set-bitmap-cmd.hh"
#include "commands/set-raster-selection-cmd.hh"
#include "geo/geo-func.hh"
#include "geo/int-rect.hh"
#include "geo/rect.hh"
#include "geo/size.hh"
#include "objects/objellipse.hh"
#include "objects/objline.hh"
#include "objects/objpath.hh"
#include "objects/objpolygon.hh"
#include "objects/objraster.hh"
#include "objects/objrectangle.hh"
#include "objects/objspline.hh"
#include "objects/objtext.hh"
#include "objects/objtri.hh"
#include "text/text-expression.hh"
#include "util-wx/clipboard.hh"
#include "util-wx/file-path-util.hh"
#include "util/at-most.hh"
#include "util/command-util.hh"
#include "util/frame-props.hh"
#include "util/grid.hh"
#include "util/image-util.hh"
#include "util/image.hh"
#include "util/index-iter.hh"
#include "util/make-vector.hh"
#include "util/object-util.hh"
#include "util/pos-info.hh"
#include "util/setting-util.hh"
#include "util/settings.hh"
#include "util/tool-util.hh"
#include "util/visit-selection.hh"
#include "python/bound-object.hh"
#include "python/python-context.hh"
#include "python/py-canvas.hh"
#include "python/py-common.hh"
#include "python/py-func-context.hh"
#include "python/py-grid.hh"
#include "python/py-less-common.hh"
#include "python/py-something.hh"
#include "python/py-tri.hh"
#include "python/py-ugly-forward.hh"
#include "python/py-util.hh"

namespace faint{

using CanvasT = const Bound<Canvas>&;

template<>
struct MappedType<CanvasT>{
  using PYTHON_TYPE = canvasObject;

  static Bound<Canvas> GetCppObject(canvasObject* self){
    return bind(*self->canvas, *self->ctx);
  }

  static bool Expired(canvasObject* self){
    return !canvas_ok(self->id, *self->ctx);
  }

  static void ShowError(canvasObject*){
    PyErr_SetString(PyExc_ValueError,
      "Operation attempted on closed canvas.");
  }
};

template<>
struct MappedType<canvasObject*>{
  // Overload to allow getting expired canvases too.
  // Pretty much only useful for expired-checks,
  // other methods should use CanvasT, not canvasObject*.

  using PYTHON_TYPE = canvasObject;

  static canvasObject* GetCppObject(canvasObject* self){
    return self;
  }

  static bool Expired(canvasObject*){
    return false;
  }

  static void ShowError(canvasObject*){}
};

bool has_object(const Image& image, ObjectId id){
  return image.Has(id);
}

bool has_object(const Canvas& canvas, ObjectId id){
  return canvas.Has(id);
}

template<typename T>
objects_t check_ownership(const T& image, const BoundObjects& boundObjects){
  auto get_obj = [&](const auto& bound){
    if (!has_object(image, bound.obj->GetId())){
      throw faint::ValueError("The canvas does not contain that item.");
    }
    return bound.obj;
  };

  return make_vector(boundObjects, get_obj);
}

bool canvas_ok(const CanvasId& c, PyFuncContext& ctx){
  return ctx.app.Exists(c);
}

static void queue_refresh(CanvasT canvas){
  canvas.ctx.py.QueueRefresh(canvas.item);
}

// Helper
static BoundObject<Object> canvas_add_object(CanvasT bc,
  Object* obj)
{
  Canvas& canvas = bc.item;
  PyFuncContext& ctx = bc.ctx;
  ctx.RunCommand(canvas, add_object_command(obj, select_added(false)));
  return bind_object(ctx, canvas, obj, canvas.GetImage().GetId());
}

/* method: "expired()\n
True if the Canvas has been closed, and this reference is dangling." */
static bool canvas_expired(canvasObject* raw){
  auto self = (canvasObject*)raw;
  return !canvas_ok(self->id, *self->ctx);
}

// Returns the base settings updated with maybe if it is set.
// Otherwise returns the base settings merged with the current tool
// settings.
static Settings specific_or_app(CanvasT canvas,
  const Settings& base,
  const Optional<Settings>& maybe)
{
  return maybe.Visit(
     [&](const Settings& s){
       return updated(base, s);
     },
     [&](){
       return updated(base, canvas.item.GetTool().GetSettings());
     });
}

/* method: "center(x,y)\n
Centers the view at image position x,y." */
static void canvas_center(CanvasT canvas, const Point& pos){
  canvas.item.CenterView(pos);
}

/* method: "clear_point_overlay()\n
Clear the point set with set_point_overlay" */
static void canvas_clear_point_overlay(CanvasT canvas){
  canvas.item.ClearPointOverlay();
  queue_refresh(canvas);
}

/* method: "context_delete()\n
Delete the selected object or raster region." */
static void canvas_context_delete(CanvasT canvas){
  context_delete(canvas, canvas.item.GetTool().GetSettings().Get(ts_Bg));
}

/* method: "context_flatten()\n
Flattens (rasterizes) all objects or all selected objects onto the
background." */
static void canvas_context_flatten(CanvasT canvas){
  canvas.ctx.RunCommand(canvas, context_flatten(canvas));
}

/* method: "context_flip_horizontal()\n
Flips the image or selection horizontally." */
static void canvas_context_flip_horizontal(CanvasT canvas){
  canvas.ctx.RunCommand(canvas, context_flip_horizontal(canvas));
}

/* method: "context_offset(dx, dy)\n
Offsets the selected objects or raster selection by dx, dy. Scrolls
the image if no selection available." */
static void canvas_context_offset(CanvasT canvas, const IntPoint& delta){
  canvas.ctx.RunCommand(canvas, context_offset(canvas, delta));
}

/* method: "context_set_alpha(v)\n
Sets the alpha of all pixels in the image or selection." */
static void canvas_context_set_alpha(CanvasT canvas, const color_value_t& alpha){
  canvas.ctx.RunCommand(canvas, context_set_alpha(canvas,
    static_cast<uchar>(alpha.GetValue())));
}

/* method: "context_flip_vertical()\n
Flips the image or selection vertically." */
static void canvas_context_flip_vertical(CanvasT canvas){
  canvas.ctx.RunCommand(canvas, context_flip_vertical(canvas));
}

/* method: "context_rotate_90CW()\n
Rotate the image or selection 90 degrees clock-wise." */
static void canvas_context_rotate_90CW(CanvasT canvas){
  canvas.ctx.RunCommand(canvas, context_rotate90cw(canvas));
}

/* method: "context_rotate(angle)\n
Rotate the image or selection by the specified number of radians.\n" */
static void canvas_context_rotate(CanvasT canvas, const Angle& angle){
  Paint bg(canvas.item.GetTool().GetSettings().Get(ts_Bg));
  canvas.ctx.RunCommand(canvas, context_rotate(canvas, angle, bg));
}

/* method: "delete_objects(objects)\n
Deletes the objects in the sequence from the image." */
static void canvas_delete_objects(CanvasT canvas,
  const Optional<BoundObjects>& maybeObjects)
{
  maybeObjects.Visit(
    [&canvas](const BoundObjects& objects){
      if (objects.empty()){
        return;
      }
      canvas.ctx.RunCommand(canvas,
        get_delete_objects_command(check_ownership(canvas, objects),
          canvas.item.GetImage()));
    });
}

/* method: "deselect(object(s))\n
Deselects the object or list of objects." */
static void canvas_deselect(CanvasT canvas,
  const Optional<BoundObjects>& objects)
{
  objects.Visit(
    [&canvas](const std::vector<BoundObject<Object>>& objects){
      canvas.item.DeselectObjects(check_ownership(canvas, objects));
      queue_refresh(canvas);
    },
    [&canvas](){
      canvas.item.DeselectObjects();
      queue_refresh(canvas);
    });
}

/* method: "dwim()\n
Alternate the behavior of the last run command (if possible)." */
static void canvas_dwim(CanvasT canvas){
  canvas.item.RunDWIM();
}

/* method: "ellipse(x,y,width,height[,settings])\n
Draw an ellipse with the current settings." */
static void canvas_ellipse(CanvasT canvas, const Rect& r,
  const Optional<Settings>& maybe)
{
  Settings s(with(specific_or_app(canvas, default_ellipse_settings(), maybe),
    ts_AntiAlias, false));

  Command* cmd = draw_object_command(
    its_yours(create_ellipse_object(tri_from_rect(r), s)));
  canvas.ctx.RunCommand(canvas, cmd);
}

/* method: "get_filename()->s\n
Returns the filename if the image has been saved, None otherwise." */
static Optional<utf8_string> canvas_get_filename(CanvasT canvas){
  Optional<FilePath> filePath(canvas.item.GetFilePath());
  if (filePath.NotSet()){
    return no_option();
  }
  return option(filePath.Get().Str());
}

/* method: "flatten(objects)\n
Flatten the specified objects onto the background." */
static void canvas_flatten(CanvasT canvas, const BoundObjects& objects){
  if (objects.empty()){
    throw TypeError("No objects specified");
  }
  const Image& image = canvas.item.GetImage();
  canvas.ctx.RunCommand(canvas,
    get_flatten_command(check_ownership(image, objects), image));
}

/* method: "flattened() -> Bitmap\n
Returns a copy of the background from this image as a Bitmap with the
active selection (if any) and all objects rasterized.\n\n

See also get_background and get_stamped_bitmap." */
static Bitmap canvas_flattened(CanvasT canvas){
  return flatten(canvas.item.GetImage());
}

/* method: "get_background()\n
Returns a copy of the pixel data in the active frame as an
ifaint.Bitmap, or none if the background is a color.\n\n

To include a floating selection, use instead get_stamped_bitmap." */
static const Optional<Bitmap>& canvas_get_background(CanvasT canvas){
  return canvas.item.GetBackground().Get<Bitmap>();
}

/* method: "get_calibration() -> ((x0,y0,x1,y1), length, unit)\n
Returns a line, its specified length and the unit this refers to - or
None if the active image is not calibrated." */
static Optional<Calibration> canvas_get_calibration(CanvasT canvas){
  return canvas.item.GetImage().GetCalibration();
}

/* method: "get_pixel((x,y))->(r,g,b,a)
Returns the background color at x, y. Note: Ignores objects." */
static Color canvas_get_pixel(CanvasT canvas, const IntPoint& pos){
  return canvas.item.GetBackground().Visit(
    [&pos](const Bitmap& bmp){
      throw_if_outside(pos, bmp);
      return get_color(bmp, pos);
    },
    [&pos](const ColorSpan& span){
      throw_if_outside(pos, span);
      return span.color;
    });
}

/* method: "get_point_overlay()->pos?\n
Returns the point overlay position set with set_point_overlay, or None
if not set." */
static Optional<IntPoint> canvas_get_point_overlay(CanvasT canvas){
  return canvas.item.GetPointOverlay();
}

/* method: "get_stamped_bitmap()\n
Returns a copy of the pixel data in the active frame as an
ifaint.Bitmap. Any floating selection will be stamped onto the
Bitmap. Objects will not be included. " */
static Bitmap canvas_get_stamped_bitmap(CanvasT canvas){
  return stamp_raster_selection(canvas.item.GetImage());
}

/* method: "get_frame([index])\n
Returns the frame with the specified index, or the selected frame if
no index specified." */
static Frame canvas_get_frame(CanvasT bc, const Optional<Index>& num){
  Canvas& canvas = bc.item;
  return num.Visit(
    [&](const Index& index){
      throw_if_outside(index, canvas.GetNumFrames());
      return Frame(bc.ctx, canvas, canvas.GetFrame(index));
    },
    [&](){
      // Return the current frame
      return Frame(bc.ctx, canvas, canvas.GetFrame(canvas.GetSelectedFrame()));
    });
}

/* method: "get_frames()->[frame1, frame2,...]\n
Returns the frames in the image." */
static std::vector<Frame> canvas_get_frames(CanvasT bc){
  Canvas& canvas = bc.item;
  return make_vector(up_to(canvas.GetNumFrames()),
    [&](const Index& i){
      return Frame(bc.ctx, canvas, canvas.GetFrame(Index(i)));
    });
}

/* method: "add_frame([w,h])\n
Adds a frame to the image, either with the optionally specified size
or the size of the currently active frame." */
static void canvas_add_frame(CanvasT canvas, const Optional<IntSize>& maybeSize){
  IntSize size = maybeSize.Or(canvas.item.GetSize());
  canvas.ctx.RunCommand(canvas, add_frame_command(size));
}

/* method: "next_frame()\n
Selects the next frame." */
static void canvas_next_frame(CanvasT canvas){
  canvas.item.NextFrame();
}

/* method: "prev_frame()\n
Selects the previous frame." */
static void canvas_prev_frame(CanvasT canvas){
  canvas.item.PreviousFrame();
}

/* method: "get_colors()->[c1, c2, ...]\n
Returns a list of the unique colors used in the active frame." */
static std::vector<Color> canvas_get_colors(CanvasT canvas){
  return canvas.item.GetImage().GetBackground().Visit(
    [](const Bitmap& bmp){
      return get_palette(bmp);
    },
    [](const ColorSpan& span) -> std::vector<Color>{
      return std::vector<Color>({span.color});
    });
}

/* method: "get_id()->canvas_id\n
Returns the id that identifies the canvas in this Faint session." */
static int canvas_get_id(CanvasT canvas){
  return canvas.item.GetId().Raw();
}

/* method: "get_max_scroll()->x,y\n
Returns the largest useful scroll positions." */
static IntPoint canvas_get_max_scroll(CanvasT canvas){
  return canvas.item.GetMaxScrollPos();
}

/* method: "get_mouse_pos()-> x, y\n
Returns the mouse position relative to the image." */
static IntPoint canvas_get_mouse_pos(CanvasT canvas){
  return floored(canvas.item.GetRelativeMousePos());
}

/* method: "get_objects()->objects\n
Returns the objects in the active frame of the image, sorted from
rear-most to front-most." */
static BoundObjects canvas_get_objects(CanvasT bc){
  Canvas& canvas = bc.item;
  const Image& image(canvas.GetImage());
  return bind_objects(bc.ctx, canvas, image.GetObjects(), image.GetId());
}

/* method: "get_paint(x,y)->paint\n
Returns the Paint at x,y. This will be the fill of the top-most item
at this position, or the background pixel color." */
static Paint canvas_get_paint(CanvasT canvas, const IntPoint& pos){
  return inside_canvas(canvas.item.GetPosInfo(pos)).Visit(
    [](const PosInside& info){
      return get_hovered_paint(info,
        include_hidden_fill(false),
        include_floating_selection(true));
    },
    []() -> Paint {
      throw ValueError("Position outside image.");
    });
}

/* method: "get_selected() -> objects\n
Returns a list of the selected objects." */
static std::vector<BoundObject<Object>> canvas_get_selected(CanvasT bc){
  Canvas& canvas = bc.item;
  const Image& image(canvas.GetImage());
  return bind_objects(bc.ctx, canvas, image.GetObjectSelection(), image.GetId());
}

/* method: "get_scroll_pos() -> x,y\n
Returns the scroll bar positions" */
static IntPoint canvas_get_scroll_pos(CanvasT canvas){
  return canvas.item.GetScrollPos();
}

/* method: "get_selection() -> (x,y,w,h)\n
Returns the selection Rectangle or None." */
static Optional<IntRect> canvas_get_selection(CanvasT canvas){
  const RasterSelection& selection = canvas.item.GetRasterSelection();
  if (selection.Empty()){
    return no_option();
  }
  return option(selection.GetRect());
}

/* method: "get_size() -> w,h\n
Returns the size of the active frame." */
static IntSize canvas_get_size(CanvasT canvas){
  return canvas.item.GetSize();
}

/* method: "get_zoom() -> f\n
Returns the current zoom as a floating point scale." */
static coord canvas_get_zoom(CanvasT canvas){
  return canvas.item.GetZoom();
}

/* method: "Ellipse((x,y,w,h)[, settings])->Ellipse\n
Adds an Ellipse object specified by the bounding box.
Uses the current tool settings if no settings specified." */
static BoundObject<Object> canvas_Ellipse(CanvasT canvas,
  const Rect& bounds,
  const Optional<Settings>& s)
{
  return canvas_add_object(canvas, create_ellipse_object(tri_from_rect(bounds),
    specific_or_app(canvas, default_ellipse_settings(), s)));
}

// Fixme: Use forwarding
/* method: "Group(object1,object2[,...])->Group\n
Adds a Group object." */
static PyObject* canvas_Group(CanvasT bc, PyObject* args){
  // Prevent empty groups
  if (PySequence_Length(args) == 0){
    PyErr_SetString(PyExc_TypeError, "A group must contain at least one object.");
    return nullptr;
  }

  // Use either the function arguments as the sequence of objects, or
  // a single sequence-argument as the sequence. i.e. allow both
  // Group(a, b, c, d) and Group([a,b,c,d])
  PyObject* sequence = (PySequence_Length(args) == 1 &&
    PySequence_Check(PySequence_GetItem(args, 0))) ?
    PySequence_GetItem(args, 0) :
    args;

  const auto n = PySequence_Length(sequence);
  // Prevent empty seguence arguments groups, i.e. Group([])
  if (n == 0){
    PyErr_SetString(PyExc_TypeError, "A group must contain at least one object.");
    return nullptr;
  }

  objects_t faintObjects;
  for (int i = 0; i != n; i++){
    PyObject* object = PySequence_GetItem(sequence, i);
    if (!PyObject_IsInstance(object, (PyObject*)&SmthType)){
      PyErr_SetString(PyExc_TypeError, "Unsupported item in list");
      return nullptr;
    }
    faintObjects.push_back(((smthObject*)object)->obj);
  }

  Canvas& canvas = bc.item;
  cmd_and_group_t p = group_objects_command(faintObjects, select_added(false));
  Command* cmd = p.first;
  Object* group = p.second;
  bc.ctx.RunCommand(Frame(bc.ctx, canvas, canvas.GetImage()), cmd);
  return pythoned(group, bc.ctx, &canvas, canvas.GetImage().GetId());
}

/* method: "Line((x0,y0,x1,y1[,...]),settings)->Line\n
Adds a Line object. The line will be a polyline if more than four coordinates
are specified." */
static BoundObject<Object> canvas_Line(CanvasT canvas,
  std::vector<coord>& coords,
  const Optional<Settings>& s)
{
  if (coords.size() < 4){
    throw ValueError("At least four coordinates required for line"
      " (x0,y0,x1,y1).");
  }
  if (coords.size() % 2 != 0){
    throw ValueError("Number of coordinates must be an even number.");
  }

  return canvas_add_object(canvas,
    create_line_object(points_from_coords(coords),
      specific_or_app(canvas, default_line_settings(), s)));
}

/* method: "Path(svg_path, settings)->Path\n
Adds a Path object described by the svg-like path bytes argument." */
static BoundObject<Object> canvas_Path(CanvasT canvas,
  const utf8_string& path,
  const Optional<Settings>& s)
{
  if (!is_ascii(path)){
    // Fixme: Consider adding ascii_string type
    throw ValueError("Non-ascii-characters in path definition.");
  }
  std::vector<PathPt> points(parse_svg_path(path.str()));
  if (points.empty()){
    throw ValueError("Failed parsing path definition.");
  }
  if (points.front().IsNotMove()){
    throw ValueError("Paths must begin with a Move-entry.");
  }

  return canvas_add_object(canvas,
    create_path_object(Points(points),
      specific_or_app(canvas, default_path_settings(), s)));
}

/* method: "Polygon((x0,y0,x1,y1...,xn,yn), settings)\n
Adds a Polygon object." */
static BoundObject<Object> canvas_Polygon(CanvasT canvas,
  const std::vector<coord>& coords,
  const Optional<Settings>& s)
{
  const size_t n = coords.size();
  if (n == 0){
    throw ValueError("No points specified.");
  }
  else if (n % 2 != 0){
    throw ValueError("Uneven number of coordinates.");
  }

  return canvas_add_object(canvas,
    create_polygon_object(points_from_coords(coords),
      specific_or_app(canvas, default_polygon_settings(), s)));
}

/* method: "Raster((x,y[,w,h]), Bitmap[, settings])\n
Adds a Raster object, scaled to the specified rectangle." */
static BoundObject<Object> canvas_Raster(CanvasT canvas,
  const Either<Point, Rect>& region,
  const Bitmap& bmp,
  const Optional<Settings>& s)
{
  Rect r = region.Visit(
    [&bmp](const Point& pt){
      return Rect(pt, floated(bmp.GetSize()));
    },
    [](const Rect& r){
      return r;
    });

  return canvas_add_object(canvas,
    new ObjRaster(tri_from_rect(r), bmp,
      specific_or_app(canvas, default_raster_settings(), s)));
}

/* method: "Rect((x,y,w,h)[, settings])->Rect\n
Adds a Rectangle object." */
static BoundObject<Object> canvas_Rect(CanvasT canvas, const Rect& r,
  const Optional<Settings>& s)
{
  return canvas_add_object(canvas,
    create_rectangle_object(tri_from_rect(r),
      specific_or_app(canvas, default_rectangle_settings(), s)));
}

/* method: "Spline((x0,y0,x1,y1,...), settings)->Spline\n
Adds a Spline object. The points are a list of coordinates sort-of
followed by the spline.\n\nFor more precise control-point handling use
Path instead." */
static BoundObject<Object> canvas_Spline(CanvasT canvas,
  const std::vector<coord>& coords,
  const Optional<Settings>& s)
{
  if (coords.empty()){
    throw ValueError("No coordinates specified.");
  }
  if (coords.size() % 2 != 0){
    throw ValueError("Uneven number of coordinates.");
  }

  return canvas_add_object(canvas,
    create_spline_object(points_from_coords(coords),
      specific_or_app(canvas, default_spline_settings(), s)));
}

/* method: "Text(pos|rect, str[, settings])->Text\n
Adds a Text object. If a rectangle is given as the first argument,
the text is bounded. " */
static BoundObject<Object> canvas_Text(CanvasT canvas,
  const Either<Rect, Point>& region,
  const utf8_string& text,
  const Optional<Settings>& maybeSettings)
{
  Settings s = specific_or_app(canvas, default_text_settings(), maybeSettings);
  Rect r = region.Visit(
    [](const Rect& r){
      return r;
    },
    [&s](const Point& pt){
      s.Set(ts_BoundedText, false);
      return Rect(pt,
        Size(100,100)); // Fixme: Just a dummy size. Should auto-size instead.
    });
  return canvas_add_object(canvas, new ObjText(tri_from_rect(r), text, s));
}

/* method: "ObjTri(tri)->TriObject\n
Faint internal, adds a TriObject."
name: "ObjTri" */
static BoundObject<Object> canvas_Tri(CanvasT canvas, const Tri& tri){
  return canvas_add_object(canvas,
    create_tri_object(tri, default_line_settings()));
}

/* method: "rect(x,y,width,height)\n
Draw a rectangle with the current settings." */
static void canvas_rect(CanvasT canvas, const Rect& r,
  const Optional<Settings>& maybe)
{

  const auto settings(with(specific_or_app(canvas,
    default_rectangle_settings(), maybe), ts_AntiAlias, false));

  Command* cmd = draw_object_command(
    its_yours(create_rectangle_object(tri_from_rect(r), settings)));

  canvas.ctx.RunCommand(canvas, cmd);
}

/* method: "redo()\n
Redo the last undone action." */
static void canvas_redo(CanvasT canvas){
  canvas.item.Redo();
}

/* method: "scroll_page_left()\n
Scrolls the image one page to the left." */
static void canvas_scroll_page_left(CanvasT canvas){
  canvas.item.ScrollPageLeft();
}

/* method: "scroll_page_right()\n
Scrolls the image one page to the right." */
static void canvas_scroll_page_right(CanvasT canvas){
  canvas.item.ScrollPageRight();
}

/* method: "scroll_page_up()\n
Scrolls the image one page up." */
static void canvas_scroll_page_up(CanvasT canvas){
  canvas.item.ScrollPageUp();
}

/* method: "scroll_page_down()\n
Scrolls the image one page down." */
static void canvas_scroll_page_down(CanvasT canvas){
  canvas.item.ScrollPageDown();
}

/* method: "scroll_max_up()\n
Scrolls the image to the top." */
static void canvas_scroll_max_up(CanvasT canvas){
  canvas.item.ScrollMaxUp();
}

/* method: "scroll_max_down()\n
Scrolls the image to the bottom." */
static void canvas_scroll_max_down(CanvasT canvas){
  canvas.item.ScrollMaxDown();
}

/* method: "scroll_max_left()\n
Scrolls the image to its left." */
static void canvas_scroll_max_left(CanvasT canvas){
  canvas.item.ScrollMaxLeft();
}

/* method: "scroll_max_right()\n
Scrolls the image to its right." */
static void canvas_scroll_max_right(CanvasT canvas){
  canvas.item.ScrollMaxRight();
}

/* method: "unsharp_mask(sigma)\n
Sharpen the image using an unsharp mask." */
static void canvas_unsharp_mask(CanvasT canvas, coord sigma){
  canvas.ctx.RunCommand(canvas,
    target_full_image(function_command("Unsharp mask",
      [=](Bitmap& bmp){
        bmp = unsharp_mask_fast(bmp, sigma);
      })));
}

/* method: "select(object1[,...])\n
Selects the object or list of objects specified.\n
The previous selection will be discarded." */
static void canvas_select(CanvasT canvas,
  const Optional<BoundObjects>& objects)
{
  objects.Visit(
    [&canvas](const BoundObjects& objects){
      canvas.item.SelectObjects(check_ownership(canvas, objects),
        deselect_old(true));
      queue_refresh(canvas);
    },
    [&canvas](){
      canvas.item.DeselectObjects();
      queue_refresh(canvas);
    });
}

/* method: "select_frame(index)\n
Selects the frame with the specified index as active." */
static void canvas_select_frame(CanvasT canvas, const Index& index){
  throw_if_outside(index, canvas.item.GetNumFrames());
  canvas.item.SelectFrame(index);
}

/* method: "set_pixel((x,y),(r,g,b,a))\n
Set the pixel at x,y to the specified color." */
static void canvas_set_pixel(CanvasT canvas,
  const IntPoint& pt,
  const Color& c)
{
  if (!point_in_image(canvas.item.GetImage(), pt)){
    throw ValueError("Point outside image.");
  }

  canvas.ctx.RunCommand(canvas, put_pixel_command(pt, c));
}

/* method: "set_point_overlay(x,y)\n
Adds a hash-mark overlay around pixel the pixel at x,y. This does not
modify the image, it's merely for visualization." */
static void canvas_set_point_overlay(CanvasT canvas, const IntPoint& pt){
  canvas.item.SetPointOverlay(pt);
  queue_refresh(canvas);
}

/* method: "set_scroll_pos(x,y)\n
Set the horizontal scroll bar position to x, and the vertical to y." */
static void canvas_set_scroll_pos(CanvasT canvas, const IntPoint& pos){
  canvas.item.SetScrollPos(pos);
}

/* method: "set_selection(x,y,w,h)\n
Set the raster selection to the given rectangle." */
static void canvas_set_selection(CanvasT canvas, const IntRect& rect){
  if (rect.x < 0 || rect.y < 0){
    throw ValueError("The x and y coordinates of the selection rectangle "
      "must be positive.");
  }
  const RasterSelection& currentSelection(canvas.item.GetRasterSelection());
  canvas.ctx.RunCommand(canvas,
    get_selection_rectangle_command(rect, currentSelection));
}

/* method: "shrink_selection()\n
Auto-shrink the selection rectangle to an image detail by trimming
same-colored areas from its sides.\n
If nothing is selected, shrink_selection will first select the entire
image and then trim." */
static void canvas_shrink_selection(CanvasT canvas){
  const RasterSelection& selection(canvas.item.GetRasterSelection());

  // Fixme: Would be better if the lambdas didn't have to
  // capture selection, as it allows mistakes down the line.
  Command* cmd = sel::visit(selection,
    [&canvas, &selection](const sel::Empty&) -> Command*{
      // No selection active: select the region that auto-crop would
      // shrink the image to.

      return canvas.item.GetBackground().Visit(
        [&selection](const Bitmap& bmp) -> Command*{
          return get_auto_crop_rectangles(bmp).Visit(
            []() -> Command*{
              return nullptr;
            },
            [&selection](const IntRect& r){
              return get_selection_rectangle_command(r, selection);
            },
            [&selection](const IntRect& r0, const IntRect& r1){
              return get_selection_rectangle_command(smallest(r0, r1),
                alternate(largest(r0, r1)), selection);
            });
        },
        [](const ColorSpan&) -> Command*{
          // Can not shrink selection with uniform background
          return nullptr;
        });
    },

    [&canvas, &selection](const sel::Rectangle& s) -> Command*{
      // Rectangle selection: shrink it by sort-of auto-cropping
      // within the selected region.

      return canvas.item.GetBackground().Get<Bitmap>().Visit(
        [&s, &selection](const Bitmap& bmp) -> Command*{
          IntRect selectionRect(s.Rect());
          Bitmap bmpSelected(subbitmap(bmp, selectionRect));
          return get_auto_crop_rectangles(bmpSelected).Visit(
            []() -> Command*{
              return nullptr;
            },
            [&](const IntRect& r){
              return get_selection_rectangle_command(
                translated(r, selectionRect.TopLeft()),
                selection);
            },
            [&](const IntRect& r0, const IntRect& r1){
              return get_selection_rectangle_command(
                translated(smallest(r0, r1), selectionRect.TopLeft()),
                alternate(translated(largest(r0, r1), selectionRect.TopLeft())),
                selection);
            });
        },
        []() -> Command*{
          return nullptr;
        });
    },

    [&selection](const sel::Moving& s) -> Command*{
      // Floating selection, shrink both the selection and the
      // hole it left.

      const Bitmap& floatingBmp(s.GetBitmap());
      return get_auto_crop_rectangles(floatingBmp).Visit(
        []() -> Command*{
          return nullptr;
        },
        [&](const IntRect& cropRect){
          Bitmap cropped(subbitmap(floatingBmp, cropRect));
          SelectionState newState(cropped,
            s.TopLeft() + cropRect.TopLeft(),
            IntRect(s.OldRect().TopLeft() + cropRect.TopLeft(),
              cropRect.GetSize()));
          return set_raster_selection_command(New(newState),
            Old(selection.GetState()), "Shrink Floating Selection", false);
        },
        [&](const IntRect& r0, const IntRect& r1){
          SelectionState newState0(subbitmap(floatingBmp, r0),
            s.TopLeft() + r0.TopLeft(),
            IntRect(s.OldRect().TopLeft() + r0.TopLeft(),
              r0.GetSize()));
          SelectionState newState1(subbitmap(floatingBmp, r1),
            s.TopLeft() + r1.TopLeft(),
            IntRect(s.OldRect().TopLeft() + r1.TopLeft(),
              r1.GetSize()));

          return set_raster_selection_command(New(newState0),
            alternate(newState1),
            Old(selection.GetState()),
            "Shrink Floating Selection",
            false);
        });
    },
    [&selection](const sel::Copying& s) -> Command*{
      // Copied selection: Shrink the floating bitmap.

      const Bitmap& floatingBmp(s.GetBitmap());
      return get_auto_crop_rectangles(floatingBmp).Visit(
        []() ->Command* {
          return nullptr;
        },
        [&](const IntRect& r){
          SelectionState newState(subbitmap(floatingBmp, r),
            s.TopLeft() + r.TopLeft());
          return set_raster_selection_command(New(newState),
            Old(selection.GetState()),
            "Shrink Floating Selection",
            false);
        },
        [&](const IntRect r0, const IntRect& r1){
          SelectionState newState0(subbitmap(floatingBmp, r0),
            s.TopLeft() + r0.TopLeft());

          SelectionState newState1(subbitmap(floatingBmp, r1),
            s.TopLeft() + r1.TopLeft());

          return set_raster_selection_command(New(newState0),
            alternate(newState1),
            Old(selection.GetState()),
            "Shrink Floating Selection", false);
        });
    });
  if (cmd != nullptr){
    canvas.ctx.RunCommand(canvas, cmd);
  }
}

/* method: "set_size((w,h)[,Paint])\n
Sets the image size to w,h, using the given Paint or the active
background color if omitted." */
static void canvas_set_size(CanvasT canvas, const IntSize& sz,
  const Optional<Paint>& maybePaint)
{
  Paint bg(maybePaint.Visit(
    [](const Paint& paint){
      return paint;
    },
    [&canvas](){
      return canvas.item.GetTool().GetSettings().Get(ts_Bg);
    }));

  canvas.ctx.RunCommand(canvas, resize_command(rect_from_size(sz), bg));
}

/* method: "swap_frames(i1, i2)\n
 Swaps the frames with indexes i1 and i2." */
static void canvas_swap_frames(CanvasT canvas, const Index& f0,
  const Index& f1)
{
  const Index numFrames(canvas.item.GetNumFrames());
  throw_if_outside(f0, numFrames);
  throw_if_outside(f1, numFrames);
  if (f0 == f1){
    return;
  }

  canvas.ctx.RunCommand(canvas,
    swap_frames_command(Index(f0), Index(f1)));
}

/* method: "test_has_bitmap()->p\n
True if the Canvas has initialized a background bitmap. Intended for
testing." */
static bool canvas_test_has_bitmap(CanvasT canvas){
  return canvas.item.GetBackground().Visit(
    [](const Bitmap&){
      return true;
    },
    [](const ColorSpan&){
      return false;
    });
}

/* method: "undo()\n
Undo the last action." */
static void canvas_undo(CanvasT canvas){
  canvas.item.Undo();
}

/* method: "zoom_default()\n
Set zoom to 1:1." */
static void canvas_zoom_default(CanvasT canvas){
  canvas.item.ZoomDefault();
}

/* method: "zoom_fit()\n
Zoom image to fit the view." */
static void canvas_zoom_fit(CanvasT canvas){
  canvas.item.ZoomFit();
}

/* method: "zoom_in()\n
Zoom in one step." */
static void canvas_zoom_in(CanvasT canvas){
  canvas.item.ZoomIn();
}

/* method: "zoom_out()\n
Zoom out one step." */
static void canvas_zoom_out(CanvasT canvas){
  canvas.item.ZoomOut();
}

/* method: "parse_text(s) ->s2\n
Expands commands on the form \\command(args..) in the text and returns
the result, using the same parsing as text objects." */
static utf8_string canvas_parse_text(CanvasT canvas, const utf8_string& text){
  return parse_text_expression(text).Visit(
    [&](const ExpressionTree& tree) -> utf8_string{
      return tree.Evaluate(canvas.item.GetImage().GetExpressionContext()).Visit(
        [](const utf8_string& result) -> utf8_string{
          return result;
        },
        [](const ExpressionEvalError& error) -> utf8_string{
          throw ValueError(error.description);
        });
    },
    [](const ExpressionParseError& error) -> utf8_string{
      throw ValueError(error.description);
    });
}

/* method: "brightness()\n
The average brightness of the image (the mean sum of the HSL lightness)." */
static coord canvas_brightness(CanvasT canvas){
  const auto& maybeBmp(canvas.item.GetBackground().Get<Bitmap>());
  if (maybeBmp.NotSet()){
    throw ValueError("Image has no bitmap.");
  }
  const Bitmap& bmp(maybeBmp.Get());
  const int w = bmp.m_w;
  const int h = bmp.m_h;
  double L = 0;
  for (int x = 0; x != w; x++){
    for (int y = 0; y != h; y++){
      HSL hsl(to_hsl(strip_alpha(get_color_raw(bmp, x,y))));
      L += hsl.l;
    }
  }
  L /= w * h;
  return L;
}

/* method: "brightness_contrast(brightness, contrast)\n
Applies brightness and contrast (aka bias and gain) to all pixels.\n\n

new_color = color * contrast + brightness.\n\n

A brightness of 0 and contrast of 1 means no change." */
static void canvas_brightness_contrast(CanvasT canvas, double brightness,
  double contrast)
{
  brightness_contrast_t values(brightness, contrast);
  canvas.ctx.RunCommand(canvas,
    target_full_image(get_brightness_and_contrast_command(values)));
}

/* property: "A Grid bound to this canvas." */
struct canvas_grid{
  static CanvasGrid Get(CanvasT self){
    return CanvasGrid(self.ctx, self.item.GetId());
  }

  static void Set(CanvasT self, const Grid& grid){
    self.item.SetGrid(grid);
    queue_refresh(self);
  }
};

/* property: "Name for the next command.\n\n

When a Python function or loop uses multiple commands to modify an
image, the commands are grouped for undo/redo with the name 'Python
commands (#)', and the number of commands specified.\n\n

If the command_name is set when the evaluation is complete, that name
will be used instead, so that a function can be given a descriptive
name." */
struct canvas_command_name{
  static utf8_string Get(CanvasT self){
    return self.ctx.py.GetCommandName(self.item);
  }

  static void Set(CanvasT self, const utf8_string& name){
    self.ctx.py.SetCommandName(self.item, name);
  }
};

// Python standard methods follow...
static PyObject* canvas_new(PyTypeObject* type, PyObject*, PyObject*){
  canvasObject *self;
  self = (canvasObject*)type->tp_alloc(type, 0);
  return (PyObject *)self;
}

static PyObject* canvas_repr(canvasObject* self){
  std::stringstream ss;
  if (canvas_ok(self->id, *self->ctx)){
    ss << "Canvas #" << self->id.Raw();
    Optional<FilePath> filePath(self->canvas->GetFilePath());
    if (filePath.IsSet()){
      ss << " " << filePath.Get().Str();
    }
  }
  else{
    ss << "Retired Canvas #" << self->id.Raw();
  }
  return build_unicode(utf8_string(ss.str()));
}

static Py_hash_t canvas_hash(PyObject* selfRaw){
  canvasObject* self((canvasObject*)selfRaw);
  return self->canvas->GetId().Raw();
}

static PyObject* canvas_richcompare(canvasObject* self, PyObject* otherRaw,
  int op)
{
  if (!PyObject_IsInstance(otherRaw, (PyObject*)&CanvasType)){
    Py_RETURN_NOTIMPLEMENTED;
  }
  canvasObject* other((canvasObject*)otherRaw);
  return py_rich_compare(self->canvas->GetId(), other->canvas->GetId(), op);
}

static void canvas_init(canvasObject&){
  throw TypeError("Canvas can not be instantiated. "
    "Use app.new or app.open instead.");
}

/* method: "__copy__() Not implemented."
name: "__copy__" */
static void canvas_copy(CanvasT){
  throw NotImplementedError("Canvas can not be copied.");
}

using common_type = CanvasT;

// Helper for py-common.hh
bool contains_pos(CanvasT canvas, const IntPoint& pos){
  if (!fully_positive(pos)){
    return false;
  }
  IntSize size = canvas.item.GetSize();
  return pos.x < size.w && pos.y < size.h;
}

void py_common_run_command(CanvasT target, Command* cmd){
  target.ctx.RunCommand(target.item, cmd);
}

/* extra_include: "generated/python/method-def/py-common-method-def.hh" */
/* extra_include: "generated/python/method-def/py-less-common-method-def.hh" */

#include "generated/python/method-def/py-canvas-method-def.hh"

PyTypeObject CanvasType = {
  PyVarObject_HEAD_INIT(nullptr, 0)
  "Canvas", // tp_name
  sizeof(canvasObject), // tp_basicsize
  0, // tp_itemsize
  nullptr, // tp_dealloc
  nullptr, // tp_print
  nullptr, // tp_getattr
  nullptr, // tp_setattr
  nullptr, // reserved (formerly tp_compare)
  (reprfunc)canvas_repr, // tp_repr
  nullptr, // tp_as_number
  nullptr, // tp_as_sequence
  nullptr, // tp_as_mapping
  canvas_hash, // tp_hash
  nullptr, // tp_call
  nullptr, // tp_str
  nullptr, // tp_getattro
  nullptr, // tp_setattro
  nullptr, // tp_as_buffer
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags
  "An image in a Faint tab.\n\nConstruction options:\n"
  " Canvas(w,h)\n"
  " Canvas(w,h,(r,g,b[,a]))\n"
  " Canvas(filename)\n\n"
  " - All opened images can be retrieved with the list_images() "
  "function or the images-list.\n", // tp_doc
  nullptr, // tp_traverse
  nullptr, // tp_clear
  (richcmpfunc)canvas_richcompare, // tp_richcompare
  0, // tp_weaklistoffset
  nullptr, // tp_iter
  nullptr, // tp_iternext
  canvas_methods, // tp_methods
  nullptr, // tp_members
  canvas_getseters, // tp_getset
  nullptr, // tp_base
  nullptr, // tp_dict
  nullptr, // tp_descr_get
  nullptr, // tp_descr_set
  0, // tp_dictoffset
  INIT_FORWARDER(canvas_init), // tp_init
  nullptr, // tp_alloc
  canvas_new, // tp_new
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

PyObject* pythoned(Canvas& canvas, PyFuncContext& ctx){
  canvasObject* py_canvas = (canvasObject*)CanvasType.tp_alloc(&CanvasType, 0);
  py_canvas->ctx = &ctx;
  py_canvas->canvas = &canvas;
  py_canvas->id = py_canvas->canvas->GetId();
  return (PyObject*) py_canvas;
}

} // namespace
