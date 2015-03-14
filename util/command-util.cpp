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
#include "bitmap/aa-line.hh"
#include "bitmap/auto-crop.hh"
#include "bitmap/color-counting.hh"
#include "bitmap/filter.hh"
#include "bitmap/quantize.hh"
#include "commands/add-object-cmd.hh"
#include "commands/change-setting-cmd.hh"
#include "commands/command-bunch.hh"
#include "commands/delete-object-cmd.hh"
#include "commands/draw-object-cmd.hh"
#include "commands/flip-rotate-cmd.hh"
#include "commands/function-cmd.hh"
#include "commands/order-object-cmd.hh"
#include "commands/rescale-cmd.hh"
#include "commands/resize-cmd.hh"
#include "commands/set-bitmap-cmd.hh"
#include "commands/set-raster-selection-cmd.hh"
#include "commands/tri-cmd.hh"
#include "geo/axis.hh"
#include "geo/geo-func.hh"
#include "geo/rect.hh"
#include "geo/scale.hh"
#include "geo/size.hh"
#include "objects/objpath.hh"
#include "objects/objraster.hh"
#include "objects/objrectangle.hh"
#include "objects/objtext.hh"
#include "text/formatting.hh"
#include "tools/tool.hh"
#include "util/at-most.hh"
#include "util/command-util.hh"
#include "util/image.hh"
#include "util/image-util.hh"
#include "util/iter.hh"
#include "util/object-util.hh"
#include "util/setting-util.hh"
#include "util/visit-selection.hh"

namespace faint{

using namespace std::placeholders; // Fixme: Replace all bind with lambda.

static Optional<Color> get_dwim_delete_color(
  const Either<Bitmap, ColorSpan>& bg,
  const IntRect& r)
{
  return bg.Visit(
    [&r](const Bitmap& bmp) -> Optional<Color>{
      color_counts_t colors;
      add_color_counts(subbitmap(bmp, IntRect(r.TopLeft(), IntSize(r.w, 1))),
        colors);
      add_color_counts(subbitmap(bmp, IntRect(r.BottomLeft(), IntSize(r.w, 1))),
        colors);
      add_color_counts(subbitmap(bmp, IntRect(r.TopLeft(), IntSize(1, r.h))),
        colors);
      add_color_counts(subbitmap(bmp, IntRect(r.TopRight(), IntSize(1, r.h))),
        colors);

      if (colors.empty()){
        return no_option(); // Can't happen
      }
      return option(most_common(colors));
    },
    [](const ColorSpan& colorSpan) -> Optional<Color>{
      return option(colorSpan.color);
    });
}

static bunch_name get_bunch_name(const utf8_string& command,
  const objects_t& objects)
{
  return bunch_name(space_sep(command, get_collective_type(objects)));
}

static bunch_name get_bunch_name(const objects_t& objects,
  const utf8_string& command)
{
  return bunch_name(space_sep(get_collective_type(objects), command));
}

// Helper for maybe_stamp_old_selection,
// SELECTION_TYPE is either sel::Copying or sel::Moving.
// C++14: Replace with generic lambda.
template<typename SELECTION_TYPE>
Command* do_stamp(const SELECTION_TYPE& selection, Command* newCommand){
  return command_bunch(CommandType::HYBRID,
    bunch_name(newCommand->Name()),
    stamp_floating_selection_command(selection),
    newCommand);
}

static Command* maybe_stamp_old_selection(Command* newCommand,
  const RasterSelection& currentSelection)
{
  return sel::visit(currentSelection,
    [&](const sel::Empty&){
      return newCommand;
    },
    [&](const sel::Rectangle&){
      return newCommand;
    },
    [&](const sel::Moving& s){
      return do_stamp(s, newCommand);
    },
    [&](const sel::Copying& s){
      return do_stamp(s, newCommand);
    });
}

Command* add_or_draw(Object* obj, Layer layer){
  return layer == Layer::RASTER ?
    draw_object_command(its_yours(obj)) :
    add_object_command(obj, select_added(false));
}

Command* crop_one_object(Object* obj){
  ObjRaster* raster = dynamic_cast<ObjRaster*>(obj);
  if (raster != nullptr){
    return crop_raster_object_command(raster);
  }
  ObjText* text = dynamic_cast<ObjText*>(obj);
  if (text != nullptr){
    return crop_text_region_command(text);
  }
  return nullptr;
}

BitmapCommand* get_aa_line_command(const IntLineSegment& line, const ColRGB& c){
  return get_function_command("Draw Wu-line",
    std::bind(draw_line_aa_Wu, _1, line, c));
}

Command* get_add_objects_command(const objects_t& objects,
  const select_added& select,
  const utf8_string& name)
{
  std::vector<Command*> commands;
  for (Object* obj : objects){
    commands.emplace_back(add_object_command(obj, select, name));
  }

  return perhaps_bunch(CommandType::OBJECT,
    get_bunch_name(name, objects), commands);
}

Command* get_auto_crop_command(const Image& image){
  const bool hasObjects = !(image.GetObjects().empty());
  auto rectangles(hasObjects ?
    get_auto_crop_rectangles(flatten(image)) :
    get_auto_crop_rectangles(image.GetBackground().Get<Bitmap>()));

  return rectangles.Visit(
    []() -> Command*{
      return nullptr;
    },
    [](const IntRect& r){
      return resize_command(r, Paint(color_white), "Auto Crop Image");
    },
    [](const IntRect& r1, const IntRect& r2){
      return resize_command(smallest(r1,r2),
        alternate(largest(r1,r2)),
        Paint(color_white),
        "Auto Crop Image");
    });
}

BitmapCommand* get_blend_alpha_command(const ColRGB& bgColor){
  return get_function_command("Replace Alpha",
    [=](Bitmap& bmp){blend_alpha(bmp, bgColor);});
}

BitmapCommand* get_sepia_command(int intensity){
  return get_function_command("Sepia", std::bind(sepia, _1, intensity));
}

BitmapCommand* get_pinch_whirl_command(coord pinch, const Angle& whirl){
  return get_function_command("Pinch/Whirl",
    std::bind(filter_pinch_whirl, _1, pinch, whirl));
}

Command* get_change_raster_background_command(ObjRaster* obj, const Color& color){
  return command_bunch(CommandType::OBJECT,
    bunch_name("Set Raster Object Background"),
    change_setting_command(obj, ts_Bg, Paint(color)),
    change_setting_command(obj, ts_BackgroundStyle, BackgroundStyle::MASKED));
}

CommandType get_collective_command_type(const commands_t& cmds){
  assert(!cmds.empty());
  CommandType type = cmds.front()->Type();
  for (const Command* cmd : but_first(cmds)){
    if (cmd->Type() != type){
      return CommandType::HYBRID;
    }
  }
  return type;
}

BitmapCommand* get_clear_command(const Paint& paint){
  return get_function_command("Clear",
    // Using lambda instead of bind because clear is overloaded
    [=](Bitmap& bmp){clear(bmp, paint);});
}

Command* get_crop_command(const objects_t& objects){
  std::vector<Command*> commands;
  for (Object* obj : objects){
    Command* cmd = crop_one_object(obj);
    if (cmd != nullptr){
      commands.push_back(cmd);
    }
  }

  if (commands.empty()){
    // No object supported autocrop
    return nullptr;
  }
  return perhaps_bunch(CommandType::OBJECT,
    get_bunch_name("Crop", objects),
    commands);
}

Command* get_crop_to_selection_command(const RasterSelection& selection,
  const Paint& bg)
{
  assert(selection.Exists());
  return resize_command(selection.GetRect(), bg, "Crop to Selection");
}

Command* get_delete_objects_command(const objects_t& objects,
  const Image& image,
  const utf8_string& name)
{
  std::vector<Command*> commands;

  // Objects must be deleted in reverse order so that the Z-order is
  // preserved.
  for (Object* obj : reversed(objects)){
    commands.push_back(delete_object_command(obj, image.GetObjectZ(obj), name));
  }
  return perhaps_bunch(CommandType::OBJECT,
    get_bunch_name(name, objects),
    commands);
}

Command* get_delete_raster_selection_command(const Image& image, const Paint& bg){
  // Fixme: Would be nicer to do sel::visit(image.GetRasterSelection())
  // and do all the work on the specific types (Floating, Moving etc),
  // but that requires adding some functionality to them.
  const RasterSelection& selection(image.GetRasterSelection());
  return sel::visit(selection,
    [](const sel::Empty&) -> Command*{
      return nullptr;
    },

    [&image,&bg](const sel::Rectangle& s) -> Command*{
      IntRect rect(intersection(image_rect(image), s.Rect()));
      if (empty(rect)){
        return nullptr;
      }

      // Try to determine a color to use for an upcoming delete of the
      // same region
      return get_dwim_delete_color(image.GetBackground(), rect).Visit(
        [&](const Color& edgeColor){
          return delete_rect_command(s.Rect(), bg,
            alternate(Paint(edgeColor)));
        },
        [&](){
          return delete_rect_command(s.Rect(), bg);
        });
    },

    [&](const sel::Moving& s){
      // Fixme: Tidy up this somehow. E.g. helper function. :)
      return command_bunch(CommandType::HYBRID,
        bunch_name("Delete Selected Region"),
        draw_object_command(its_yours(create_rectangle_object(tri_from_rect(floated(s.OldRect())),
              eraser_rectangle_settings(s.GetOptions().bg)))),
        set_raster_selection_command(New(SelectionState()),
          Old(selection.GetState()), ""));
    },

    [&](const sel::Copying&){
      return set_raster_selection_command(New(SelectionState()),
        Old(selection.GetState()),
        "Delete Floating Selection");
    });
}

Command* get_deselect_raster_command(const sel::Existing& selection){
  return sel::visit(selection,
    [&](const sel::Rectangle&)->Command*{
      // Just deselect
      return set_raster_selection_command(New(SelectionState()),
        Old(selection.GetState()), "Deselect Raster");
    },

    [&](const sel::Moving& moving)->Command*{
      return command_bunch(CommandType::HYBRID,
        bunch_name("Deselect Raster (moved)"),
        stamp_floating_selection_command(moving),
        set_raster_selection_command(New(SelectionState()),
          Old(selection.GetState()), ""));
    },

    [&](const sel::Copying& copying)->Command*{
      return command_bunch(CommandType::HYBRID,
        bunch_name("Deselect Raster (copied)"),
        stamp_floating_selection_command(copying),
        set_raster_selection_command(New(SelectionState()),
          Old(selection.GetState()), ""));
    });
}

Command* get_insert_raster_bitmap_command(const Bitmap& bmp,
  const IntPoint& topLeft,
  const RasterSelection& oldSelection,
  const Settings& s,
  const utf8_string& commandName)
{
  return maybe_stamp_old_selection(
    command_bunch(CommandType::HYBRID, bunch_name(commandName),
      set_selection_options_command(New(SelectionOptions(masked_background(s),
          s.Get(ts_Bg), s.Get(ts_AlphaBlending))),
        Old(oldSelection.GetOptions())),
      set_raster_selection_command(New(SelectionState(bmp, topLeft)),
        Old(oldSelection.GetState()), "")),
    oldSelection);
}

BitmapCommand* get_pixelize_command(const pixelize_range_t& width){
  return get_function_command("Pixelize", std::bind(pixelize, _1, width));
}

BitmapCommand* get_quantize_command(){
  return get_function_command("Quantize",
    [=](Bitmap& bmp){
      quantize(bmp, Dithering::ON);
    });
}

Command* get_selection_rectangle_command(const IntRect& r,
  const RasterSelection& currentSelection)
{
  Command* selectRectangle =
    set_raster_selection_command(New(SelectionState(r)),
      Old(currentSelection.GetState()),
      "Select Rectangle");
  return maybe_stamp_old_selection(selectRectangle, currentSelection);
}

Command* get_selection_rectangle_command(const IntRect& r,
  const Alternative<IntRect>& altRect,
  const RasterSelection& currentSelection)
{
  Command* selectRectangle =
    set_raster_selection_command(New(SelectionState(r)),
      alternate(SelectionState(altRect.Get())),
      Old(currentSelection.GetState()),
      "Select Rectangle");
  return maybe_stamp_old_selection(selectRectangle, currentSelection);
}

Command* get_select_all_command(const Image& image,
  const RasterSelection& currentSelection)
{
  Command* selectAll =
    set_raster_selection_command(New(SelectionState(image_rect(image))),
    Old(currentSelection.GetState()),
    "Select All");
  return maybe_stamp_old_selection(selectAll, currentSelection);
}

BitmapCommand* get_set_alpha_command(uchar alpha){
  return get_function_command("Set Alpha", std::bind(set_alpha, _1, alpha));
}

BitmapCommand* get_desaturate_simple_command(){
  return get_function_command("Desaturate", desaturate_simple);
}

BitmapCommand* get_desaturate_weighted_command(){
  return get_function_command("Desaturate Weighted", desaturate_weighted);
}

BitmapCommand* get_erase_but_color_command(const Color& keep, const Paint& eraser){
  return get_function_command("Replace Colors",
    std::bind(erase_but, _1, keep, eraser));
}

Command* get_fill_boundary_command(Object* obj, const Paint& paint){
  return change_setting_command(obj, ts_Fg, paint);
}

Command* get_fill_inside_command(Object* obj, const Paint& paint){
  const Settings& s(obj->GetSettings());
  commands_t commands;
  FillStyle fillStyle = s.Get(ts_FillStyle);
  if (!filled(fillStyle)){
    // Change the object to filled
    fillStyle = with_fill(fillStyle);
    commands.push_back(change_setting_command(obj, ts_FillStyle, fillStyle));
  }

  commands.push_back(change_setting_command(obj,
      setting_used_for_fill(fillStyle), paint));

  return perhaps_bunch(CommandType::OBJECT,
    bunch_name("Set Object Fill Color"), commands);
}

Command* get_flatten_command(const objects_t& objects, const Image& image){
  assert(!objects.empty());
  std::deque<Command*> commands;
  for (Object* obj : objects){
    commands.push_back(draw_object_command(just_a_loan(obj)));
    commands.push_front(delete_object_command(obj, image.GetObjectZ(obj)));
  }

  return command_bunch(CommandType::HYBRID, get_bunch_name("Flatten", objects),
    commands);
}

BitmapCommand* get_flood_fill_command(const IntPoint& pos, const Paint& fill){
  return get_function_command("Flood fill", std::bind(flood_fill, _1, pos, fill));
}

BitmapCommand* get_boundary_fill_command(const IntPoint& pos, const Paint& fill,
  const Color& boundary)
{
  return get_function_command("Boundary fill",
    std::bind(boundary_fill, _1, pos, fill, boundary));
}

BitmapCommand* get_brightness_and_contrast_command(const brightness_contrast_t& v){
  return get_function_command("Brightness and contrast",
    [=](Bitmap& bmp){bmp = brightness_and_contrast(bmp, v);});
}

BitmapCommand* get_invert_command(){
  return get_function_command("Invert colors",
    [=](Bitmap& bmp){invert(bmp);});
}

Command* get_move_objects_command(const objects_t& objects,
  const NewTris& in_newTris,
  const OldTris& in_oldTris)
{
  const std::vector<Tri>& oldTris(in_oldTris.Get());
  const std::vector<Tri>& newTris(in_newTris.Get());
  assert(!objects.empty());
  assert(oldTris.size() == newTris.size() && newTris.size() == objects.size());

  commands_t commands;
  for (size_t i = 0; i != objects.size(); i++){
    commands.push_back(new TriCommand(objects[i],
      New(newTris[i]),
      Old(oldTris[i]), "Move"));
  }

  return perhaps_bunch(CommandType::OBJECT,
    get_bunch_name("Move", objects),
    commands);
}

class MergeIfSameObjects : public MergeCondition{
public:
  MergeIfSameObjects(const objects_t& objects)
    : m_objects(objects)
  {}

  bool Satisfied(MergeCondition* cond) override{
    MergeIfSameObjects* candidate = dynamic_cast<MergeIfSameObjects*>(cond);
    if (candidate == nullptr){
      return false;
    }
    // Fixme: Verify object identity safety
    return m_objects == candidate->m_objects;
  }

  bool Append(Command*) override{
    // MergeIfSameObjects is not used for appending
    return false;
  }

  bool AssumeName() const override{
    return false;
  }

  MergeIfSameObjects& operator=(const MergeIfSameObjects&) = delete;

private:
  const objects_t m_objects;
};

Command* get_offset_objects_command(const objects_t& objects, const Point& delta){
  assert(!objects.empty());

  commands_t commands;
  for (Object* obj : objects){
    Tri tri = obj->GetTri();
    commands.push_back(new TriCommand(obj,
      New(translated(tri, delta.x, delta.y)),
      Old(tri),
      "Offset",
      MergeMode::SOCIABLE));
  }

  if (commands.size() == 1){
    return commands.back();
  }

  return command_bunch(CommandType::OBJECT,
    get_bunch_name("Offset", objects), commands, new MergeIfSameObjects(objects));
}

Command* perhaps_bunched_reorder(const std::vector<Command*>& commands,
  const objects_t& objects,
  const NewZ& newZ,
  const OldZ& oldZ)
{
  if (commands.empty()){
    // No object could be moved
    return nullptr;
  }
  else if (commands.size() == 1){
    return commands.back();
  }
  const utf8_string objectsName(get_collective_type(objects));
  const utf8_string dirStr = forward_or_back_str(newZ, oldZ);
  const bunch_name commandName(space_sep(objectsName,dirStr));
  return command_bunch(CommandType::OBJECT, commandName, commands);
}

Command* get_objects_backward_command(const objects_t& objects,
  const Image& image)
{
  assert(!objects.empty());
  int zLimit = 0;
  std::vector<Command*> commands;
  for (Object* object : objects){
    int Z = image.GetObjectZ(object);

    if (Z == 0 || Z - 1 < zLimit){
      // Prevent the current object from moving below Z-depth 0, and
      // from moving below an object that was previously prevented to
      // move below Z=0 (etc.), so that the relative order of all
      // selected objects is retained.
      zLimit = Z + 1;
    }
    else {
      zLimit = Z - 1;
      commands.push_back(order_object_command(object, New(Z - 1), Old(Z)));
    }
  }
  return perhaps_bunched_reorder(commands, objects, New(0), Old(1));
}

Command* get_objects_forward_command(const objects_t& objects,
  const Image& image)
{
  assert(!objects.empty());
  std::vector<Command*> commands;
  int zLimit = resigned(image.GetObjects().size());
  for (Object* object : reversed(objects)){
    int Z = image.GetObjectZ(object);

    // Prevent objects from being moved on top of other selected objects that
    // could not be moved further up because of reaching top.
    if (Z + 1 >= zLimit){
      zLimit = Z;
    }
    else {
      zLimit = Z + 1;
      commands.push_back(order_object_command(object, New(Z + 1), Old(Z)));
    }
  }
  return perhaps_bunched_reorder(commands, objects, New(1), Old(0));
}

Command* get_objects_to_back_command(const objects_t& objects,
  const Image& image)
{
  std::vector<Command*> commands;
  const int numObjects = resigned(objects.size());
  for (int i = 0; i != numObjects; i++){
    Object* object = objects[to_size_t(i)];
    int currentPos = image.GetObjectZ(object);
    if (i != currentPos){
      commands.push_back(order_object_command(object, New(i), Old(currentPos)));
    }
  }
  return perhaps_bunched_reorder(commands, objects, New(0), Old(1));
}

Command* get_objects_to_front_command(const objects_t& objects,
  const Image& image)
{
  assert(!objects.empty());
  const int maxPos = get_highest_z(image);
  std::vector<Command*> commands;
  for (Object* obj : objects){
    const int currentPos = image.GetObjectZ(obj);
    if (maxPos != currentPos){
      commands.push_back(order_object_command(obj, New(maxPos), Old(currentPos)));
    }
  }
  return perhaps_bunched_reorder(commands, objects, New(1), Old(0));
}

Command* get_objects_to_paths_command(const objects_t& objects,
  const Image& image,
  const select_added& selectAdded)
{
  std::vector<Command*> commands;

  // Delete the replaced objects from front-most to rear-most to
  // maintain valid Z-ordering
  for (Object* obj : objects){
    commands.push_back(get_replace_object_command(Old(obj),
        clone_as_path(obj, image.GetExpressionContext()), image, selectAdded));
  }
  return perhaps_bunch(CommandType::OBJECT,
    get_bunch_name(objects, objects.size() == 1 ? "to Path" : "to Paths"),
    commands);
}

BitmapCommand* get_replace_color_command(const OldColor& oldColor,
  const Paint& newColor)
{
  return get_function_command("Replace color",
    std::bind(replace_color, _1, oldColor, newColor));
}

Command* get_replace_object_command(const OldObject& oldWrap,
  Object* newObject,
  const Image& image,
  const select_added& selectAdded)
{
  Object* old(oldWrap.Get());
  auto z = image.GetObjectZ(old);
  Command* removeCommand = delete_object_command(old, z, "");
  Command* insertCommand = add_object_command(newObject, selectAdded, z, "");

  return command_bunch(CommandType::OBJECT,
    bunch_name(space_sep(old->GetType(), "to", newObject->GetType())),
    removeCommand,
    insertCommand);
}

Command* get_resize_command(const Optional<Bitmap>& bmp,
  const IntRect& rect,
  const Paint& paint)
{
  if (bmp.IsSet()){
    Optional<Color> altCol = resize_dwim_color(bmp.Get(), rect);
    if (altCol.IsSet() && altCol.Get() != paint){
      return resize_command(rect, paint, alternate(Paint(altCol.Get())));
    }
  }
  return resize_command(rect, paint);
}

Command* get_rotate_command(Object* obj, const Angle& angle,
  const Point& origin)
{
  const Tri& tri = obj->GetTri();
  return new TriCommand(obj,
    New(rotated(tri, angle, origin)), Old(tri), "Rotate");
}

Command* get_rotate_command(const objects_t& objects, const Angle& angle,
  const Point& origin)
{
  std::vector<Command*> commands;
  for (Object* obj : objects){
    commands.push_back(get_rotate_command(obj, angle, origin));
  }

  return perhaps_bunch(CommandType::OBJECT,
    get_bunch_name("Rotate", objects), commands);
}

Command* get_rotate_selection_command(const Image& image){
  auto selection(image.GetRasterSelection());
  using State = SelectionState;
  return set_raster_selection_command(
    New(sel::visit(selection,
      [](const sel::Empty&){
        assert(false);
        return State();
      },
      [&image](const sel::Rectangle& s){
        return State(rotate_90cw(subbitmap(image, s.Rect())),
          s.TopLeft(), s.Rect());
       },
       [](const sel::Moving& s){
         return State(rotate_90cw(s.GetBitmap()),
           s.TopLeft(), s.OldRect());
       },
       [](const sel::Copying& s){
         return State(rotate_90cw(s.GetBitmap()),
           s.TopLeft());
       })),
    Old(selection.GetState()),
    "Rotate Selection Clockwise");
}

Command* get_rotate_selection_command(const Image& image,
  const Angle& angle,
  const Paint& bg)
{
  auto selection(image.GetRasterSelection());
  using State = SelectionState;
  return set_raster_selection_command(
    New(sel::visit(selection,
      [](const sel::Empty&){
        assert(false);
        return State();
      },
      [&](const sel::Rectangle& s){
        return State(
          rotate_bilinear(subbitmap(image, s.Rect()), angle, bg),
          s.TopLeft(), s.Rect());
      },
      [&](const sel::Moving& s){
        Bitmap bmp(rotate_bilinear(s.GetBitmap(), angle, bg));
        return State(bmp, selection.TopLeft(), selection.GetOldRect());
      },
      [&](const sel::Copying& s){
        Bitmap bmp(rotate_bilinear(s.GetBitmap(), angle, bg));
        return State(bmp, selection.TopLeft());
      })),
    Old(selection.GetState()),
    "Rotate Selection");
}

class AppendIfMoveCommand : public MergeCondition{
// For grouping undo/redo of consecutive selection movements
// with keyboard (get_offset_raster_selection_command).
public:
  bool Append(Command* cmd) override{
    return is_move_raster_selection_command(cmd);
  }
  bool AssumeName() const override{
    return false;
  }
  bool Satisfied(MergeCondition*) override{
    return false;
  }
};

Command* get_offset_raster_selection_command(const Image& image,
  const IntPoint& delta)
{
  auto selection(image.GetRasterSelection());
  return sel::visit(selection,
    [](const sel::Empty&) -> Command*{
      assert(false);
      return nullptr;
    },
    [&](const sel::Rectangle& s){
      // Begin floating, and offset
      SelectionState newState(subbitmap(image, s.Rect()),
        s.TopLeft() + delta, s.Rect());

      return command_bunch(CommandType::HYBRID,
        bunch_name("Offset Raster Selection"),
        set_raster_selection_command(New(newState), Old(selection.GetState()),
          ""),
        new AppendIfMoveCommand());
    },

    [&delta](const sel::Moving& s){
      const IntPoint oldPos = s.TopLeft();
      return move_raster_selection_command(oldPos + delta, oldPos);
    },

    [&delta](const sel::Copying& s){
      const IntPoint oldPos = s.TopLeft();
      return move_raster_selection_command(oldPos + delta, oldPos);
    });
}

Command* get_scale_command(const objects_t& objects, const Scale& scale,
  const Point& origin)
{
  std::vector<Command*> commands;
  for (Object* obj : objects){
    const Tri& tri = obj->GetTri();
    commands.push_back(new TriCommand(obj,
        New(scaled(tri, scale, origin)), Old(tri), "Scale"));
  }
  return perhaps_bunch(CommandType::OBJECT,
    get_bunch_name("Scale", objects),
    commands);
}

Command* get_scale_rotate_command(const objects_t& objects, const Scale& scale,
  const Angle& angle, const Point& origin)
{
  std::vector<Command*> commands;
  for (Object* obj : objects){
    const Tri& oldTri = obj->GetTri();
    Tri newTri(rotated(scaled(oldTri, scale, origin), angle, origin));
    commands.push_back(new TriCommand(obj,
        New(newTri), Old(oldTri), "Scale and rotate"));
  }
  return perhaps_bunch(CommandType::OBJECT,
    get_bunch_name("Scale and rotate", objects),
    commands);
}

BitmapCommand* get_threshold_command(const threshold_range_t& range,
  const Paint& in, const Paint& out)
{
  return get_function_command("Threshold",
    std::bind(threshold, _1, range, in, out));
}
Command* get_scale_raster_selection_command(const Image& image,
  const IntSize& newSize, ScaleQuality quality)
{
  auto selection(image.GetRasterSelection());
  const Scale scaling(New(floated(newSize)), floated(selection.GetSize()));
  return set_raster_selection_command(
    New(sel::visit(selection,
      [](const sel::Empty&){
        assert(false);
        return SelectionState();
      },
      [&](const sel::Rectangle& s){
        // Fixme: Initialize mask settings and what not
        Bitmap scaled(scale(subbitmap(image, s.Rect()), scaling, quality));
        return SelectionState(scaled, s.TopLeft(), s.Rect());
      },
      [&](const sel::Moving& s){
        return SelectionState(scale(s.GetBitmap(), scaling, quality),
          s.TopLeft(), s.OldRect());
      },
      [&](const sel::Copying& s){
        return SelectionState(scale(s.GetBitmap(), scaling, quality),
          selection.TopLeft());
      })),
    Old(selection.GetState()),
    "Scale Selection");
}

OperationFlip::OperationFlip(Axis axis)
  : m_axis(axis)
{}

Command* OperationFlip::DoImage() const{
  return flip_image_command(m_axis);
}

Command* OperationFlip::DoObjects(const objects_t& objects) const{
  coord xScale = m_axis == Axis::HORIZONTAL ? -1.0 : 1.0;
  Scale scale(xScale, -xScale);
  Point origin = bounding_rect(objects).Center();
  std::vector<Command*> commands;
  for (Object* obj : objects){
    const Tri& tri = obj->GetTri();
    commands.push_back(new TriCommand(obj, New(scaled(tri, scale, origin)),
      Old(tri), "Flip"));
  }
  return perhaps_bunch(CommandType::OBJECT, get_bunch_name("Flip", objects),
    commands);
}

Command* OperationFlip::DoRasterSelection(const Image& image) const{
  auto selection(image.GetRasterSelection());
  return sel::visit(selection,
    [](const sel::Empty&) -> Command*{
      assert(false);
      return nullptr;
    },

    [&](const sel::Rectangle&){
      SelectionState newState(flip(subbitmap(image, selection.GetRect()),
          along(m_axis)),
        selection.TopLeft(), selection.GetRect());

      return set_raster_selection_command(New(newState),
        Old(selection.GetState()),
        space_sep("Flip Selection", str_axis_adverb(m_axis)));
    },

    [&](const sel::Moving&){
      // Fixme: Mem use, reversible - consider saving the process
      // instead of saving bitmap.
      Bitmap bmp(flip(selection.GetBitmap(), along(m_axis)));
      SelectionState newState(bmp, selection.TopLeft(),
        selection.GetOldRect());
      return set_raster_selection_command(New(newState),
        Old(selection.GetState()),
        space_sep("Flip Selection", str_axis_adverb(m_axis)));
    },

    [&](const sel::Copying&){
      Bitmap bmp(flip(selection.GetBitmap(), along(m_axis)));
      SelectionState newState(bmp, selection.TopLeft());
      return set_raster_selection_command(New(newState),
        Old(selection.GetState()),
        space_sep("Flip Selection", str_axis_adverb(m_axis)));
    });
}

} // namespace faint
