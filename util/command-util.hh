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

#ifndef FAINT_COMMAND_UTIL_HH
#define FAINT_COMMAND_UTIL_HH
#include <deque>
#include "bitmap/bitmap-fwd.hh"
#include "bitmap/draw.hh" // OldColor.. Todo: Move?
#include "bitmap/filter.hh"
#include "commands/add-object-cmd.hh"
#include "commands/bitmap-cmd.hh"
#include "commands/command.hh"
#include "commands/delete-rect-cmd.hh"
#include "geo/tri.hh"
#include "util/common-fwd.hh"
#include "util/setting-id.hh"

namespace faint{

class ObjRaster;

// Gets an add object or a draw object command depending on the layer
Command* add_or_draw(Object*, Layer);

// Experimental custom-anti-aliased line
BitmapCommand* get_aa_line_command(const IntLineSegment&, const ColRGB&);

Command* get_add_objects_command(const objects_t&,
  const select_added&,
  const utf8_string& name="Add");

Command* get_auto_crop_command(const Image&);

BitmapCommand* get_blend_alpha_command(const ColRGB& bgColor);

BitmapCommand* get_sepia_command(int);

Command* get_change_raster_background_command(ObjRaster*, const Color&);

// Returns RASTER if all commands are raster, OBJECT if all are object
// or HYBRID if any is hybrid or there are both object and raster
// commands.
CommandType get_collective_command_type(const commands_t&);

BitmapCommand* get_clear_command(const Paint&);

// Returns a command which crops any croppable objects passed in or 0
// if no object could be cropped.
Command* get_crop_command(const objects_t&);

Command* get_crop_to_selection_command(const RasterSelection&, const Paint& bg);

Command* get_delete_objects_command(const objects_t&, const Image&,
  const utf8_string& name="Delete");

// Returns a command that deletes the raster selection.
// Asserts that the image has a raster selection.
Command* get_delete_raster_selection_command(const Image&, const Paint& bgCol);

BitmapCommand* get_desaturate_simple_command();

BitmapCommand* get_desaturate_weighted_command();

BitmapCommand* get_erase_but_color_command(const Color& keep, const Paint& eraser);

Command* get_fill_boundary_command(Object*, const Paint&);

Command* get_fill_inside_command(Object*, const Paint&);

Command* get_flatten_command(const objects_t&, const Image&);

BitmapCommand* get_flood_fill_command(const IntPoint&, const Paint&);

BitmapCommand* get_boundary_fill_command(const IntPoint&,
  const Paint& fill, const Color& boundary);

BitmapCommand* get_brightness_and_contrast_command(const brightness_contrast_t&);

BitmapCommand* get_invert_command();

using NewTris = Order<tris_t>::New;
using OldTris = Order<tris_t>::Old;

Command* get_move_objects_command(const objects_t&,
  const NewTris&,
  const OldTris&);

Command* get_offset_objects_command(const objects_t&, const Point& delta);

Command* get_offset_raster_selection_command(const Image&, const IntPoint& delta);

Command* get_objects_backward_command(const objects_t&, const Image&);
Command* get_objects_forward_command(const objects_t&, const Image&);
Command* get_objects_to_back_command(const objects_t&, const Image&);
Command* get_objects_to_front_command(const objects_t&, const Image&);

// Inserts the bitmap floating at the given position.
// Stamps the old selection if necessary.
Command* get_insert_raster_bitmap_command(const Bitmap&,
  const IntPoint&,
  const RasterSelection& oldSelection,
  const Settings&,
  const utf8_string& commandName);

Command* get_objects_to_paths_command(const objects_t&,
  const Image&,
  const select_added&);

BitmapCommand* get_pinch_whirl_command(coord, const Angle&);
BitmapCommand* get_pixelize_command(const pixelize_range_t&);

BitmapCommand* get_quantize_command();

BitmapCommand* get_replace_color_command(const OldColor&, const Paint&);

using OldObject = Order<Object*>::Old;
Command* get_replace_object_command(const OldObject&, Object*, const Image&, const select_added&);

BitmapCommand* get_set_alpha_command(uchar alpha);

// Returns a ResizeCommand which uses the bgCol as background if expanding,
// possibly with a DWIM-option using the edge-color from the expand directions.
Command* get_resize_command(const Optional<Bitmap>&,
  const IntRect& newRect,
  const Paint& bg);

Command* get_rotate_command(Object*, const Angle&, const Point& origin);

Command* get_rotate_command(const objects_t&, const Angle&, const Point& origin);

Command* get_scale_rotate_command(const objects_t&,
  const Scale&,
  const Angle&,
  const Point& origin);

Command* get_scale_command(const objects_t&,
  const Scale&,
  const Point& origin);

BitmapCommand* get_threshold_command(const threshold_range_t& range,
  const Paint& in,
  const Paint& out);

Command* get_deselect_raster_command(const sel::Existing&);

Command* get_selection_rectangle_command(const IntRect&,
  const RasterSelection& current);

Command* get_selection_rectangle_command(const IntRect&,
  const Alternative<IntRect>&,
  const RasterSelection& current);

Command* get_select_all_command(const Image&, const RasterSelection&);

Command* get_rotate_selection_command(const Image&);

Command* get_rotate_selection_command(const Image&,
  const Angle& angle,
  const Paint& bg);

Command* get_offset_selection_command(const Image&, const IntPoint& delta);

Command* get_scale_raster_selection_command(const Image&,
  const IntSize&,
  ScaleQuality);

class OperationFlip : public Operation {
public:
  OperationFlip(Axis);
  Command* DoImage() const override;
  Command* DoObjects(const objects_t&) const override;
  Command* DoRasterSelection(const Image&) const override;
private:
  Axis m_axis;
};

} // namespace

#endif
