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

#include "app/canvas.hh"
#include "app/context-commands.hh"
#include "app/get-app-context.hh"
#include "bitmap/auto-crop.hh" // Fixme: Move most-common-yada to color-counting
#include "commands/flip-rotate-cmd.hh"
#include "commands/group-objects-cmd.hh"
#include "geo/axis.hh"
#include "geo/geo-func.hh"
#include "geo/rect.hh"
#include "geo/scale.hh"
#include "geo/size.hh"
#include "tools/tool-id.hh"
#include "util/apply-target.hh"
#include "util/command-util.hh"
#include "util/image.hh"
#include "util/object-util.hh"
#include "util/visit-selection.hh"

namespace faint{

Command* context_targetted(BitmapCommand* cmd, Canvas& canvas){
  return sel::visit(canvas.GetRasterSelection(),
    [=](const sel::Empty&){
      return target_full_image(cmd);
    },
    [=](const sel::Rectangle& s){
      return target_rectangle(cmd, s.Rect());
    },
    [=](const sel::Floating&){
      return target_floating_selection(cmd);
    });
}

Command* get_apply_command(const Canvas& canvas, const Operation& op){
  return dispatch_target(get_apply_target(canvas),
    [&](OBJECT_SELECTION){
      return op.DoObjects(canvas.GetImage().GetObjectSelection());
    },
    [&](RASTER_SELECTION){
      return op.DoRasterSelection(canvas.GetImage());
    },
    [&](IMAGE){
      return op.DoImage();
    });
}

static Command* crop_to_raster_selection_command(const Image& image){
  const RasterSelection& selection(image.GetRasterSelection());
  if (!selection.Exists()){
    return nullptr;
  }
  return get_crop_to_selection_command(selection,
    get_app_context().GetToolSettings().Get(ts_Bg));
}

Command* context_crop(Canvas& canvas){
  const Image& active(canvas.GetImage());
  Layer layer = canvas.GetTool().GetLayerType();
  if (layer == Layer::RASTER){
    Command* cmd = crop_to_raster_selection_command(active);
    if (cmd != nullptr){
      return cmd;
    }
  }
  else {
    // Object layer
    Command* cmd = get_crop_command(active.GetObjectSelection());
    if (cmd != nullptr){
      return cmd;
    }
  }

  // No active raster selections and no croppable objects selected, do
  // an auto-crop
  return get_auto_crop_command(active);
}

void context_delete(Canvas& canvas, const Paint& bg){
  ToolInterface& tool = canvas.GetTool();

  if (tool.SupportsSelection()){
    tool.Delete();
    return;
  }

  if (tool.GetLayerType() == Layer::OBJECT){
    const Image& image = canvas.GetImage();
    const objects_t& objects = image.GetObjectSelection();
    if (objects.empty()){
      return;
    }
    canvas.RunCommand(get_delete_objects_command(objects, image));
  }
  else if (tool.GetLayerType() == Layer::RASTER){
    Command* cmd = get_delete_raster_selection_command(canvas.GetImage(), bg);
    if (cmd != nullptr){
      canvas.RunCommand(cmd);
    }
  }
  canvas.Refresh();
}

Command* context_deselect(Canvas& canvas){
  ToolInterface& tool(canvas.GetTool());
  if (tool.Deselect()){
    return nullptr;
  }

  Layer layer = tool.GetLayerType();
  if (layer == Layer::RASTER){
    const RasterSelection& selection(canvas.GetImage().GetRasterSelection());
    return selection.Exists() ?
      get_deselect_raster_command(sel::Existing(selection)) :
      nullptr;
  }
  else {
    canvas.DeselectObjects();
    canvas.Refresh(); // Fixme: Tricky, required, because no command is used
    return nullptr;
  }
}

Command* context_flatten(Canvas& canvas){
  const Image& active = canvas.GetImage();
  const objects_t& allObjects = active.GetObjects();
  if (allObjects.empty()){
    // No objects to flatten.
    return nullptr;
  }
  const objects_t& selected = active.GetObjectSelection();
  return get_flatten_command(selected.empty() ?
    allObjects : selected,
    active);
}

Command* context_flip_horizontal(const Canvas& canvas){
  return get_apply_command(canvas, OperationFlip(Axis::HORIZONTAL));
}

Command* context_flip_vertical(const Canvas& canvas){
  return get_apply_command(canvas, OperationFlip(Axis::VERTICAL));
}

Command* context_offset(Canvas& canvas, const IntPoint& delta){
  return dispatch_target(get_apply_target(canvas),
    [&](OBJECT_SELECTION){
      const objects_t& objectSelection(canvas.GetImage().GetObjectSelection());
      return get_offset_objects_command(objectSelection,
        floated(delta));
    },
    [&](RASTER_SELECTION){
      return get_offset_raster_selection_command(canvas.GetImage(), delta);
    },
    [&](IMAGE) -> Command*{
      canvas.SetScrollPos(canvas.GetScrollPos() + delta);
      return nullptr;
    });
}

Command* context_rotate90cw(const Canvas& canvas){
  return dispatch_target(get_apply_target(canvas),
    [&](OBJECT_SELECTION){
      const objects_t& objectSelection(canvas.GetImage().GetObjectSelection());
      Point origin = bounding_rect(objectSelection).Center();
      return get_rotate_command(objectSelection,
        pi / 2,
        origin);
    },

    [&](RASTER_SELECTION){
      return get_rotate_selection_command(canvas.GetImage());
    },

    [&](IMAGE){
      return rotate_image_90cw_command();
    });
}

Command* context_rotate(const Canvas& canvas,
  const Angle& angle,
  const Paint& bg)
{
  return dispatch_target(get_apply_target(canvas),
    [&](OBJECT_SELECTION){
      const objects_t& objectSelection(canvas.GetImage().GetObjectSelection());
      Point origin = bounding_rect(objectSelection).Center();
      return get_rotate_command(objectSelection, angle, origin);
    },

    [&](RASTER_SELECTION){
      return get_rotate_selection_command(canvas.GetImage(),
        angle, bg);
    },

    [&](IMAGE){
      const Bitmap& bmp(canvas.GetBackground().Expect<Bitmap>());
      auto altBg(alternate(Paint(most_common_edge_color(bmp))));
      return rotate_image_command(angle, bg, altBg);
    });
}

Command* context_select_all(Canvas& canvas){
  ToolInterface& tool(canvas.GetTool());

  if (tool.SelectAll()){
    // Selection within a tool is not handled by a command
    return nullptr;
  }

  Layer layer = tool.GetLayerType();
  if (layer == Layer::RASTER){
    const Image& active(canvas.GetImage());
    get_app_context().SelectTool(ToolId::SELECTION); // Fixme
    return get_select_all_command(active,
      active.GetRasterSelection());
  }
  else {
    canvas.SelectObjects(canvas.GetObjects(), deselect_old(true));
    get_app_context().SelectTool(ToolId::SELECTION);
    // Object selection is not handled with commands
    return nullptr;
  }
}

Command* context_set_alpha(Canvas& canvas, uchar alpha){
  BitmapCommand* cmd = get_set_alpha_command(alpha);
  return context_targetted(cmd, canvas);
}

Command* context_scale_objects(const Canvas& canvas, const Size& size){
  // Fixme: Make this take the list of objects instead
  const Image& active = canvas.GetImage();
  const objects_t& objectSelection(active.GetObjectSelection());
  Rect oldRect = bounding_rect(objectSelection);
  Point origin = oldRect.TopLeft();
  Scale scale(New(size), oldRect.GetSize());
  return get_scale_command(objectSelection, scale, origin);
}

Command* group_selected_objects(Canvas& canvas){
  const Image& active(canvas.GetImage());
  const objects_t& objectSelection(active.GetObjectSelection());

  if (objectSelection.size() <= 1){
    // Need at least two objects to group
    return nullptr;
  }
  cmd_and_group_t cmd =
    group_objects_command(objectSelection, select_added(true));
  return cmd.first;
}

Command* context_objects_backward(Canvas& canvas){
  const Image& active(canvas.GetImage());
  const objects_t& objectSelection(active.GetObjectSelection());
  if (objectSelection.empty()){
    return nullptr;
  }
  return get_objects_backward_command(objectSelection, active);
}

Command* context_objects_forward(Canvas& canvas){
  const Image& active(canvas.GetImage());
  const objects_t& objectSelection(active.GetObjectSelection());
  if (objectSelection.empty()){
    return nullptr;
  }
  return get_objects_forward_command(objectSelection, active);
}

Command* context_objects_to_front(Canvas& canvas){
  const Image& active(canvas.GetImage());
  const objects_t& objectSelection(active.GetObjectSelection());
  if (objectSelection.empty()){
    return nullptr;
  }
  return get_objects_to_front_command(objectSelection, active);
}

Command* context_objects_to_path(Canvas& canvas){
  const Image& image(canvas.GetImage());
  const objects_t& selection(image.GetObjectSelection());
  if (selection.empty()){
    return nullptr;
  }

  return get_objects_to_paths_command(selection, image, select_added(true));
}

Command* context_objects_to_back(Canvas& canvas){
  const Image& active(canvas.GetImage());
  const objects_t& objectSelection(active.GetObjectSelection());
  if (objectSelection.empty()){
    return nullptr;
  }
  return get_objects_to_back_command(objectSelection, active);
}

Command* ungroup_selected_objects(Canvas& canvas){
  const Image& active(canvas.GetImage());
  objects_t groups = get_groups(active.GetObjectSelection());
  if (groups.empty()){
    return nullptr;
  }
  return ungroup_objects_command(groups, select_added(true));
}

}  // namespace
