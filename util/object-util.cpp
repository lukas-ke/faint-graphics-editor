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
#include <cassert>
#include "bitmap/paint.hh"
#include "bitmap/pattern.hh"
#include "geo/geo-func.hh"
#include "geo/measure.hh"
#include "geo/points.hh"
#include "objects/objellipse.hh"
#include "objects/objline.hh"
#include "objects/objpath.hh"
#include "objects/objraster.hh"
#include "objects/objtext.hh"
#include "rendering/faint-dc.hh"
#include "text/utf8-string.hh"
#include "util/default-settings.hh"
#include "util/grid.hh"
#include "util/iter.hh"
#include "util/math-constants.hh"
#include "util/object-util.hh"
#include "util/setting-util.hh"

namespace faint{

objects_t as_list(Object* obj){
  objects_t objects;
  objects.push_back(obj);
  return objects;
}

ObjRaster* as_ObjRaster(Object* obj){
  ObjRaster* raster = dynamic_cast<ObjRaster*>(obj);
  assert(raster != nullptr);
  return raster;
}

Rect bounding_rect(const objects_t& objects){
  assert(!objects.empty());
  Tri t(objects[0]->GetTri());
  Point minPt = min_coords(t.P0(), t.P1(), t.P2(), t.P3());
  Point maxPt = max_coords(t.P0(), t.P1(), t.P2(), t.P3());
  for (const Object* obj : but_first(objects)){
    t = obj->GetTri();
    minPt = min_coords(minPt, min_coords(t.P0(), t.P1(), t.P2(), t.P3()));
    maxPt = max_coords(maxPt, max_coords(t.P0(), t.P1(), t.P2(), t.P3()));
  }
  return Rect(minPt, maxPt);
}

Rect bounding_rect(const Tri& tri, const Settings& s){
  if (border(s)){
    // Pad by half the line width (the line is centered on object
    // edges)
    coord padding = s.Get(ts_LineWidth) / 2;
    return inflated(bounding_rect(tri), padding);
  }
  return bounding_rect(tri);
}

objects_t clone(const objects_t& oldObjects){
  objects_t objects;
  for (const Object* obj : oldObjects){
    objects.push_back(obj->Clone());
  }
  return objects;
}

Object* clone_as_path(Object* object, const ExpressionContext& ctx){
  Settings s(default_path_settings());
  s.Update(object->GetSettings());

  if (is_text(object)){
    // Texts have no fill setting - but are drawn like filled paths
    // using their primary color. Setting the path's fillstyle like
    // this makes the path more similar to the source text.
    s.Set(ts_FillStyle, FillStyle::FILL);
  }
  else if (is_raster_object(object)){
    // Create a pattern from the raster object's bitmap
    ObjRaster* rasterObj = dynamic_cast<ObjRaster*>(object);
    s.Set(ts_FillStyle, FillStyle::FILL);
    Pattern pattern(rasterObj->GetBitmap(),
      IntPoint(0,0), object_aligned_t(true));
    s.Set(ts_Fg, Paint(pattern));
  }

  s.Set(ts_EditPoints, true);
  Points pts(object->GetPath(ctx));
  return create_path_object(pts, s);
}

Color color_at(ObjRaster* obj, const Point& imagePos){
  Bitmap bmp(IntSize(1,1),
    get_color_default(obj->GetSettings().Get(ts_Bg), color_white));
  FaintDC dc(bmp);
  dc.SetOrigin(-imagePos);
  obj->Draw(dc);
  return get_color(bmp, IntPoint(0,0));
}

bool contains(const objects_t& objects, const Object* obj){
  return std::find(begin(objects), end(objects), obj) != end(objects);
}

bool contains_group(const objects_t& objects){
  for (Object* obj : objects){
    if (obj->GetObjectCount() > 0){
      return true;
    }
  }
  return false;
}

size_t find_object_index(Object* obj, const objects_t& objects){
  assert(obj != nullptr);
  for (size_t i = 0; i != objects.size(); i++){
    if (objects[i] == obj){
      return i;
    }
  }
  return objects.size();
}

std::vector<Point> get_attach_points(const Tri& tri){
  return {tri.P0(),
      tri.P1(),
      tri.P2(),
      tri.P3(),
      mid_P0_P1(tri),
      mid_P0_P2(tri),
      mid_P1_P3(tri),
      mid_P2_P3(tri),
      center_point(tri)};
}

Object* get_by_name(Object* obj, const utf8_string& name){
  if (obj->GetName() == name){
    return obj;
  }
  const int numObjects = obj->GetObjectCount();
  for (int i = 0; i != numObjects; i++){
    Object* subObj = get_by_name(obj->GetObject(i), name);
    if (subObj != nullptr){
      return subObj;
    }
  }
  return nullptr;
}

Object* get_by_name(const objects_t& objects, const utf8_string& name){
  for (Object* obj : objects){
    Object* match = get_by_name(obj, name);
    if (match != nullptr){
      return match;
    }
  }
  return nullptr;
}

utf8_string get_collective_type(const objects_t& objects){
  assert(!objects.empty());
  utf8_string first = objects.front()->GetType();
  if (objects.size() == 1){
    return first;
  }

  for (const Object* obj : but_first(objects)){
    if (obj->GetType() != first){
      return "Objects";
    }
  }
  return first + "s"; // Naive plural
}

objects_t get_groups(const objects_t& objects){
  objects_t groups;
  for (Object* obj : objects){
    if (obj->GetObjectCount() != 0){
      groups.push_back(obj);
    }
  }
  return groups;
}

objects_t get_intersected(const objects_t& objects, const Rect& r){
  objects_t intersected;
  for (Object* obj : objects){
    if (intersects(bounding_rect(obj->GetTri()), r)){
      intersected.push_back(obj);
    }
  }
  return intersected;
}

Settings get_object_settings(const objects_t& objects){
  Settings s;
  for (Object* obj : objects){
    s.UpdateAll(obj->GetSettings());
  }
  return s;
}

tris_t get_tris(const objects_t& objects){
  tris_t tris;
  for (Object* obj : objects){
    tris.push_back(obj->GetTri());
  }
  return tris;
}

bool is_or_has(const Object* object, const ObjectId& id){
  if (object->GetId() == id){
    return true;
  }
  for (int i = 0; i != object->GetObjectCount(); i++){
    if (is_or_has(object->GetObject(i), id)){
      return true;
    }
  }
  return false;
}

bool is_raster_object(Object* obj){
  return obj != nullptr && dynamic_cast<ObjRaster*>(obj) != nullptr;
}

bool is_rotated(Object* obj){
  return !rather_zero(obj->GetTri().GetAngle());
}

bool is_text_object(const Object* obj){
  return dynamic_cast<const ObjText*>(obj) != nullptr;
}

bool lacks(const objects_t& objects, const Object* obj){
  return std::find(begin(objects), end(objects), obj) == end(objects);
}

Point next_point(Object* obj, int index){
  const int end = obj->NumPoints();
  assert(index < end);
  if (index != end - 1){
    return obj->GetPoint(index + 1);
  }
  return obj->CyclicPoints() ?
    obj->GetPoint(0) :
    obj->GetPoint(end - 2);
}

bool object_aligned_resize(const Object* obj){
  return obj->GetSettings().GetDefault(ts_AlignedResize, false);
}

void offset_by(Object* obj, const Point& d){
  obj->SetTri(translated(obj->GetTri(), d.x, d.y));
}

void offset_by(Object* obj, const IntPoint& d){
  offset_by(obj, floated(d));
}

void offset_by(const objects_t& objects, const Point& d){
  for (Object* obj : objects){
    offset_by(obj, d);
  }
}

void offset_by(const objects_t& objects, const IntPoint& d){
  offset_by(objects, floated(d));
}

coord object_area(const Object* obj){
  if (is_ellipse(obj)){
    // Fixme: Handle arcs
    Tri t = obj->GetTri();
    return math::pi * (t.Width() / 2) * (t.Height() / 2);
  }
  if (is_line(obj)){
    // No area for you.
    return 0.0;
  }
  // Fixme: Only correct for rectangle. :)
  return area(obj->GetTri());
}

coord perimeter(const Object* obj, const ExpressionContext& ctx){
  if (is_ellipse(obj)){
    Tri tri = obj->GetTri();
    return ellipse_perimeter(tri.Width(), tri.Height());
  }
  if (is_text(obj)){
    return 0.0;
  }

  auto path(obj->GetPath(ctx));
  assert(!path.empty());
  return perimeter(path);
}

bool point_edit_disabled(const Object* obj){
  return !point_edit_enabled(obj);
}

bool point_edit_enabled(const Object* obj){
  return obj->GetSettings().GetDefault(ts_EditPoints, false);
}

Point prev_point(Object* obj, int index){
  const int end = obj->NumPoints();
  assert(index < end);
  if (index != 0){
    return obj->GetPoint(index - 1);
  }
  return obj->CyclicPoints() ?
    obj->GetPoint(end - 1) :
    obj->GetPoint(index + 1); // Fixme: Fishy.
}

bool remove(const Object* obj, const from_t<objects_t>& from){
  objects_t& objects(from.Get());
  auto obj_eq = [&obj](Object* x){return x == obj;};
  objects_t::iterator newEnd = std::remove_if(begin(objects), end(objects),
    obj_eq);

  if (newEnd == end(objects)){
    return false;
  }
  objects.erase(newEnd, end(objects));
  return true;
}

static bool remove_objects_from(objects_t& objects, const objects_t& remove){
  auto should_remove =
    [&remove](const Object* obj){return contains(remove, obj);};

  objects_t::iterator newEnd = std::remove_if(begin(objects), end(objects),
    should_remove);

  if (newEnd == end(objects)){
    return false;
  }
  objects.erase(newEnd, end(objects));
  return true;
}

bool remove(const objects_t& remove, const from_t<objects_t>& objects){
  return remove_objects_from(objects.Get(), remove);
}

bool resize_handles_enabled(const Object* obj){
  return point_edit_disabled(obj);
}

ColorSetting setting_used_for_fill(const Object* obj){
  const Settings& s(obj->GetSettings());
  assert(s.Has(ts_FillStyle));
  return s.Get(ts_FillStyle) == FillStyle::FILL ?
    ts_Fg : ts_Bg;
}

const coord g_maxSnapDistance = 20.0;
Point snap(const Point& sourcePt,
  const objects_t& objects,
  const Grid& grid,
  coord maxSnapDistance)
{
  std::vector<Point> noExtraPoints;
  return snap(sourcePt, objects, grid, noExtraPoints, maxSnapDistance);
}

Point snap(const Point& sourcePt,
  const objects_t& objects,
  const Grid& grid,
  const std::vector<Point>& extraPoints,
  coord maxSnapDistance)
{
  coord lastSnapDistance = maxSnapDistance;
  Point currentPt(sourcePt);
  for (const Object* obj : objects){
    for (const Point& pt : obj->GetAttachPoints()){
      coord snapDistance = distance(sourcePt, pt);
      if (snapDistance < lastSnapDistance){
        // Snap to this closer point instead
        lastSnapDistance = snapDistance;
        currentPt = pt;
      }
    }
  }

  if (grid.Enabled()){
    Point gridPoint = grid.Snap(sourcePt);
    coord snapDistance = distance(sourcePt, gridPoint);
    if (snapDistance < lastSnapDistance){
      lastSnapDistance = snapDistance;
      currentPt = gridPoint;
    }
  }

  for (const Point& pt : extraPoints){
    coord snapDistance = distance(sourcePt, pt);
    if (snapDistance < lastSnapDistance){
      // Snap to this closer point instead
      lastSnapDistance = snapDistance;
      currentPt = pt;
    }
  }
  return currentPt;
}

coord snap_x(coord sourceX,
  const objects_t& objects,
  const Grid& grid,
  coord y0,
  coord y1,
  coord maxSnapDistance)
{
  coord lastSnapDistance = maxSnapDistance;
  coord current(sourceX);
  for (const Object* obj : objects){
    for (const Point& pt : obj->GetAttachPoints()){
      if (y0 <= pt.y && pt.y <= y1){
        coord snapDistance = std::fabs(pt.x - sourceX);
        if (lastSnapDistance > snapDistance){
          // Snap to this closer point instead
          lastSnapDistance = snapDistance;
          current = pt.x;
        }
      }
    }
  }

  if (grid.Enabled()){
    Point gridPoint = grid.Snap(Point(sourceX, 0));
    coord snapDistance = std::fabs(sourceX - gridPoint.x);
    if (snapDistance < lastSnapDistance){
      current = gridPoint.x;
    }
  }
  return current;
}

coord snap_y(coord sourceY,
  const objects_t& objects,
  const Grid& grid,
  coord x0,
  coord x1,
  coord maxSnapDistance)
{
  coord lastSnapDistance = maxSnapDistance;
  coord current(sourceY);
  for (Object* obj : objects){
    for (const Point& pt : obj->GetAttachPoints()){
      if (x0 <= pt.x && pt.x <= x1){
        coord snapDistance = std::fabs(pt.y - sourceY);
        if (lastSnapDistance > snapDistance){
          // Snap to this closer point instead
          lastSnapDistance = snapDistance;
          current = pt.y;
        }
      }
    }
  }
  if (grid.Enabled()){
    Point gridPoint = grid.Snap(Point(0, sourceY));
    coord snapDistance = fabs(sourceY - gridPoint.y);
    if (snapDistance < lastSnapDistance){
      current = gridPoint.y;
    }
  }
  return current;
}

bool supports_object_aligned_resize(Object* object){
  return object->GetSettings().Has(ts_AlignedResize);
}

bool supports_point_editing(Object* object){
  return object->GetSettings().Has(ts_EditPoints);
}

bool toggle_object_aligned_resize(Object* object){
  const Settings& settings = object->GetSettings();
  if (!settings.Has(ts_AlignedResize)){
    return false;
  }
  object->Set(ts_AlignedResize, settings.Not(ts_AlignedResize));
  return true;
}

bool toggle_edit_points(Object* object){
  const Settings& settings = object->GetSettings();
  if (!settings.Has(ts_EditPoints)){
    return false;
  }
  object->Set(ts_EditPoints, !settings.Get(ts_EditPoints));
  return true;
}

} // namespace
