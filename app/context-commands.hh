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

Command* context_targetted(BitmapCommand*, Canvas&);

// If the layer type is the raster layer and there's a raster
// selection the image will be cropped to the selection.
//
// If the layer type is the object layer, any selected
// croppable objects will be cropped.
//
// If neither objects or raster cropping happened, auto-crop is
// attempted.
Command* context_crop(Canvas&);

// Deletes the raster or object selection (if any). Fills a hole left
// by a raster selection with the Paint
void context_delete(Canvas&, const Paint&);

// Tool, object or raster deselect
Command* context_deselect(Canvas&);

// Flattens the selected objects, or all objects if none are selected.
// Does nothing if there are no objects.
Command* context_flatten(Canvas&);

Command* context_flip_horizontal(const Canvas&);

Command* context_flip_vertical(const Canvas&);

Command* context_objects_backward(Canvas&);

Command* context_objects_forward(Canvas&);

Command* context_objects_to_back(Canvas&);

Command* context_objects_to_front(Canvas&);

Command* context_objects_to_path(Canvas&);

Command* context_offset(Canvas&, const IntPoint& delta);

Command* context_rotate90cw(const Canvas&);

Command* context_rotate(const Canvas&, const Angle&, const Paint& bg);

Command* context_scale_objects(const Canvas&, const Size&);

Command* context_select_all(Canvas&);

Command* context_set_alpha(Canvas&, uchar alpha);

Command* get_apply_command(const Canvas&, const Operation&);

// Groups the selected objects. Does nothing if no objects are
// selected.
Command* group_selected_objects(Canvas&);

// Ungroups the selected groups. Does nothing if no groups are
// selected.
Command* ungroup_selected_objects(Canvas&);

} // namespace
#endif
