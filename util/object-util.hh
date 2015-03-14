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

#ifndef FAINT_OBJECT_UTIL_HH
#define FAINT_OBJECT_UTIL_HH
#include "util/distinct.hh"
#include "util/id-types.hh"
#include "util/objects.hh"
#include "util/tris.hh"

namespace faint{

class Color;
class ExpressionContext;
class Grid;
class ObjRaster;
class Point;
class Rect;

objects_t as_list(Object*);
ObjRaster* as_ObjRaster(Object*);
Rect bounding_rect(const objects_t&);

// Returns a bounding rectangle for the Tri, padded with the line
// width from the Settings if the fill style includes border.
Rect bounding_rect(const Tri&, const Settings&);

objects_t clone(const objects_t&);

// Creates a new Path object based on the passed in object.
Object* clone_as_path(Object*, const ExpressionContext&);

// Returns the color at imagePos, which is relative to the image (not
// the ObjRaster!). Will return the ts_Bg of the ObjRaster if the
// imagePos is outside the object.  Ignores masking, so if imagePos
// falls on a region in the object which is transparent due to being
// the masked background color, the color will still be returned.
Color color_at(ObjRaster*, const Point& imagePos);

// true if atleast one object has sub-objects
bool contains_group(const objects_t&);

bool contains(const objects_t&, const Object*);

// Returns the object index, or the vector size if not found.
size_t find_object_index(Object*, const objects_t&);
std::vector<Point> get_attach_points(const Tri&);

// Returns obj if the name matches or the first sub-object
// for which name matches or nullptr if nothing matched.
Object* get_by_name(Object*, const utf8_string& name);

// Returns the first object matching name, including objects
// within groups.
Object* get_by_name(const objects_t&, const utf8_string& name);

objects_t get_groups(const objects_t&);
objects_t get_intersected(const objects_t&, const Rect&);

// Returns the exact type of a single object (e.g. "Line"), the
// naively-pluralized type for multiple same-type objects
// (e.g. "Lines"), and the generic "Objects" for mixes.
utf8_string get_collective_type(const objects_t&);

Settings get_object_settings(const objects_t&);
tris_t get_tris(const objects_t&);

// True if the the object or one of its sub-objects match the id
bool is_or_has(const Object*, const ObjectId&);
bool is_raster_object(Object*);
bool is_rotated(Object*);
bool is_text_object(const Object*);

bool lacks(const objects_t&, const Object*);

coord object_area(const Object*);

// Returns the point with the next index, possibly wrapping or doing
// other magic stuff.
Point next_point(Object*, int index);
bool object_aligned_resize(const Object*);
void offset_by(Object*, const Point& delta);
void offset_by(Object*, const IntPoint& delta);
void offset_by(const objects_t&, const Point& delta);
void offset_by(const objects_t&, const IntPoint& delta);

coord perimeter(const Object* object, const ExpressionContext&);

bool point_edit_disabled(const Object*);

// True if the objects points should be adjustable. This should be
// checked by tools that care about movable and extension points.
bool point_edit_enabled(const Object*);

// Returns the point with previous index, possibly wrapping or doing
// other magic stuff.
Point prev_point(Object*, int index);

bool remove(const Object*, const from_t<objects_t>&);
bool remove(const objects_t&, const from_t<objects_t>&);

// Whether the objects resize handles are shown/usable.
bool resize_handles_enabled(const Object*);

extern const coord g_maxSnapDistance;
Point snap(const Point&, const objects_t&, const Grid&,
  coord maxDistance=g_maxSnapDistance);

Point snap(const Point&, const objects_t&, const Grid&,
  const std::vector<Point>& extraPoints, coord maxDistance=g_maxSnapDistance);

coord snap_x(coord x, const objects_t&, const Grid&, coord y0, coord y1,
  coord maxDistance=g_maxSnapDistance);
coord snap_y(coord y, const objects_t&, const Grid&, coord x0, coord x1,
  coord maxDistance=g_maxSnapDistance);

bool supports_object_aligned_resize(Object*);
bool supports_point_editing(Object*);

// Toggles ts_AlignedResize for the object and returns true, or false
// if the the object does not support aligned resize.
bool toggle_object_aligned_resize(Object*);

// Toggles ts_EditPoints for the object and returns true, or false if
// the object does not support point editing.
bool toggle_edit_points(Object*);

} // namespace

#endif
