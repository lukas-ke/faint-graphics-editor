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

#ifndef FAINT_CONTEXT_COMMANDS_HH
#define FAINT_CONTEXT_COMMANDS_HH
#include "app/app-getter-util.hh"
#include "geo/angle.hh"
#include "geo/primitive.hh"

namespace faint{

// Functions for creating commands depending on the current canvas
// state - typically the current selection or the active tool.

class BitmapCommand;
class Canvas;
class Command;
class IntPoint;
class Operation;
class Paint;
class Size;

CommandPtr context_targetted(BitmapCommand*, Canvas&);

// If the layer type is the raster layer and there's a raster
// selection the image will be cropped to the selection.
//
// If the layer type is the object layer, any selected
// croppable objects will be cropped.
//
// If neither objects or raster cropping happened, auto-crop is
// attempted.
CommandPtr context_crop(Canvas&, const Paint& bg);

// Deletes the raster or object selection (if any). Fills a hole left
// by a raster selection with the Paint
void context_delete(Canvas&, const Paint&);

// Tool, object or raster deselect
CommandPtr context_deselect(Canvas&);

// Flattens the selected objects, or all objects if none are selected.
// Does nothing if there are no objects.
CommandPtr context_flatten(Canvas&);

CommandPtr context_flip_horizontal(const Canvas&);

CommandPtr context_flip_vertical(const Canvas&);

CommandPtr context_objects_backward(Canvas&);

CommandPtr context_objects_forward(Canvas&);

CommandPtr context_objects_to_back(Canvas&);

CommandPtr context_objects_to_front(Canvas&);

CommandPtr context_objects_to_path(Canvas&);

CommandPtr context_offset(Canvas&, const IntPoint& delta);

CommandPtr context_pixel_snap(Canvas&);

CommandPtr context_rotate90cw(const Canvas&);

CommandPtr context_rotate(const Canvas&, const Angle&, const Paint& bg);

CommandPtr context_scale_objects(const Canvas&, const Size&);

CommandPtr context_select_all(Canvas&, const change_tool_f&);

CommandPtr context_set_alpha(Canvas&, uchar alpha);

CommandPtr get_apply_command(const Canvas&, const Operation&);

// Groups the selected objects. Does nothing if no objects are
// selected.
CommandPtr group_selected_objects(Canvas&);

// Ungroups the selected groups. Does nothing if no groups are
// selected.
CommandPtr ungroup_selected_objects(Canvas&);

} // namespace

#endif
