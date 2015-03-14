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

#include <cassert>
#include "app/active-canvas.hh"
#include "bitmap/color.hh"
#include "commands/add-object-cmd.hh"
#include "commands/add-point-cmd.hh"
#include "commands/command-bunch.hh"
#include "commands/move-point-cmd.hh"
#include "commands/remove-point-cmd.hh"
#include "commands/tri-cmd.hh"
#include "geo/measure.hh"
#include "objects/object.hh"
#include "rendering/overlay.hh"
#include "tasks/select-object-idle.hh"
#include "tasks/select-object-move-point.hh"
#include "tasks/select-object-move.hh"
#include "tasks/select-object-rectangle.hh"
#include "tasks/select-object-resize.hh"
#include "tasks/select-object-rotate.hh"
#include "tasks/select-object-set-name.hh"
#include "tasks/standard-task.hh"
#include "tasks/text-edit.hh"
#include "text/formatting.hh"
#include "util/command-util.hh"
#include "util/convenience.hh"
#include "util/object-util.hh"
#include "util/tool-util.hh"
#include "util/pos-info.hh"
#include "util-wx/key-codes.hh"

namespace faint{

template<typename T>
class AppendCommandType : public MergeCondition{
  // Condition for appending a command of a certain type to a
  // command-bunch.
public:
  AppendCommandType()
    : m_appended(false)
  {}

  bool Satisfied(MergeCondition*) override{
    // AppendCommandType is only used for appending, not merging
    // CommandBunches.
    return false;
  }

  bool Append(Command* cmd) override{
    if (m_appended){
      // Only append a single command
      return false;
    }
    m_appended = true; // Only attempt appending once

    T* candidate = dynamic_cast<T*>(cmd);
    if (candidate == nullptr){
      return false;
    }
    return true;
  }

  bool AssumeName() const override{
    return false;
  }
private:
  bool m_appended;
};

static bool is_drag(const Point& p0, const Point& p1){
  return distance(p0, p1) >= 3;
}

static bool not_drag(const Point& p0, const Point& p1){
  return !is_drag(p0,p1);
}

static bool mod_clone_rotate_toggle(const PosInfo& info){
  return info.modifiers.Primary();
}

static bool (&mod_clone)(const PosInfo&) = mod_clone_rotate_toggle;
static bool (&mod_rotate)(const PosInfo&) = mod_clone_rotate_toggle;
static bool (&mod_toggle_select)(const PosInfo&) = mod_clone_rotate_toggle;

static bool mod_select_multi_add(const PosInfo& info){
  return info.modifiers.Secondary();
}

static utf8_string object_type(const PosInfo& info){
  assert(info.object != nullptr);
  return decapitalized(info.object->GetType());
}

static utf8_string status_for_object(const PosInfo& info){
  if (info.handle.IsSet()){
    return info.handle.Get().Visit(
      [&](const Handle& handle) -> utf8_string{
        if (corner_handle(handle)){
          return info.modifiers.Primary() ?
            Sentence("Click to rotate the", object_type(info)) :
            Sentence("Click to resize the", object_type(info) +
              primary_modifier("Rotate"));
        }
        else{
          return Sentence("Click to resize the", object_type(info));
        }
      },
      [&](const std::pair<object_handle_t, HandleType>& handle){
        HandleType handleType = handle.second;
        if (handleType == HandleType::MOVABLE_POINT){
          return info.object->CanRemovePoint() ?
            "Left Click=Move, Right Click=Delete." :
            "Left click to move the point.";
        }
        else if(handleType == HandleType::EXTENSION_POINT){
          return "Left Click and Drag to add a point.";
        }
        else{
          assert(false);
          return "";
        }
      });
  }
  else if (hovered_selected_text(info, SearchMode::include_grouped) != nullptr){
    return "Double Click to edit the text";
  }
  else if (info.objSelected && info.hitStatus == Hit::INSIDE){
    if (supports_point_editing(info.object)){
      return point_edit_enabled(info.object) ?
       "Double click to disable point editing." :
       "Double click to enable point editing.";
    }
    if (supports_object_aligned_resize(info.object) && is_rotated(info.object)){
      return object_aligned_resize(info.object) ?
        "Right click to enable image aligned resize." :
        "Right click to enable object aligned resize.";
    }
  }
  else if (info.object != nullptr && !info.objSelected){
    return Sentence("Left click to select the", object_type(info));
  }
  else if (info.object == nullptr){
    return "Left click to draw an object-selection rectangle.";
  }
  return "";
}

struct ClickedObject{
  // Keeps track of whether a clicked object was selected when clicked
  Object* object;
  bool wasSelected;
};

static bool quick_draggable(const ClickedObject& clicked){
  // An object that wasn't selected can be instantly dragged, saving a
  // click to select it first.
  return clicked.object != nullptr && !clicked.wasSelected;
}

static bool (&selectable)(const ClickedObject&) = quick_draggable;

static bool deselectable(const ClickedObject& clicked){
  // For the clicked object to be deselectable, it must have been selected
  // already when it was clicked.
  return clicked.object != nullptr && clicked.wasSelected;
}

static bool no_object(const ClickedObject& clicked){
  return clicked.object == 0;
}

class category_select_object_idle;
// Whether an object was selected when clicked
using was_selected = Distinct<bool, category_select_object_idle, 0>;

struct IdleSelectionState{
public:
  IdleSelectionState(Settings& settings, const ActiveCanvas& canvas)
    : canvas(canvas),
      settings(settings)
  {
    Reset();
  }

  void Reset(bool in_fullRefresh=false){
    clickPos = Point(0,0);
    mouseDown = false;
    fullRefresh = in_fullRefresh;
    newTask.Set(nullptr);
    command.Set(nullptr);
    clicked.object = nullptr;
    clicked.wasSelected = false;
  }

  void SetClickedObject(Object* obj, const Point& pos,
    const was_selected& wasSelected)
  {
    clicked.object = obj;
    clicked.wasSelected = wasSelected.Get();
    clickPos = pos;
    mouseDown = true;
  }

  ActiveCanvas canvas;
  Settings& settings;
  Point clickPos;
  bool mouseDown;
  mutable bool fullRefresh;
  PendingTask newTask;
  PendingCommand command;
  ClickedObject clicked;

  IdleSelectionState& operator=(const IdleSelectionState&) = delete;
};

static TaskResult clicked_nothing(IdleSelectionState& impl, const PosInfo& info){
  impl.mouseDown = true;
  impl.clickPos = info.pos;
  if (mod_clone_rotate_toggle(info) || mod_select_multi_add(info)){
    // Do not deselect objects if a modifier is held, as this probably
    // means either that the user mis-clicked or intends to draw a
    // selection or deselection-rectangle
    impl.mouseDown = true;
    impl.clickPos = info.pos;
    return TaskResult::NONE;
  }

  // Clicked outside object - deselect objects
  info.canvas.DeselectObjects();
  impl.fullRefresh = true;
  return TaskResult::DRAW;
}

static Command* get_appending_insert_command(Object* obj,
  object_handle_t handle,
  const Point& pos)
{
  Command* cmd = add_point_command(obj, handle + 1, pos);
  return command_bunch(cmd->Type(), bunch_name(cmd->Name()), cmd,
    new AppendCommandType<MovePointCommand>());
}

static Command* get_appending_add_object_command(Object* obj,
  const utf8_string& commandName)
{
  Command* cmd = add_object_command(obj, select_added(true), commandName);
  return command_bunch(cmd->Type(), bunch_name(cmd->Name()), cmd,
    new AppendCommandType<TriCommand>());
}

static TaskResult clicked_selected(IdleSelectionState& impl, const PosInfo& info){
  Object* obj(info.object);
  if (info.handle.IsSet()){
    return info.handle.Get().Visit(
      [&](const Handle& handle){
        bool copy = info.modifiers.RightMouse();
        if (mod_rotate(info) && corner_handle(handle)){
          // Rotate the object around the corner handle
          if (copy){
            Object* newObject = obj->Clone();
            impl.command.Set(get_appending_add_object_command(newObject,
              "Rotate Clone"));
            impl.newTask.Set(rotate_object_task(newObject,
              info.handle.Get().Expect<Handle>()));
            return TaskResult::COMMIT_AND_CHANGE;
          }
          else{
            impl.newTask.Set(rotate_object_task(obj, info.handle.Get().Expect<Handle>()));
            return TaskResult::CHANGE;
          }
        }
        else {
          if (copy){
            Object* newObject = obj->Clone();
            impl.command.Set(get_appending_add_object_command(newObject, "Resize Clone"));
            impl.newTask.Set(select_object_resize(newObject,
              info.handle.Get().Expect<Handle>()));
            return TaskResult::COMMIT_AND_CHANGE;
          }
          else{
            impl.newTask.Set(select_object_resize(obj,
              info.handle.Get().Expect<Handle>()));
          }
        }
        return TaskResult::CHANGE;
      },
      [&](const std::pair<object_handle_t, HandleType>& handle){
        if (handle.second == HandleType::MOVABLE_POINT && point_edit_enabled(obj)){
          bool rightMouse = info.modifiers.RightMouse();
          // Right mouse means remove point for extendable objects
          if (rightMouse && obj->Extendable()){
            // Delete a point if possible
            if (obj->CanRemovePoint()){
              impl.command.Set(remove_point_command(obj, handle.first));
              return TaskResult::COMMIT;
            }
            // Not possible to delete points for some reason (e.g. at
            // minimum number of points) - do nothing.
            return TaskResult::NONE;
          }
          else{
            impl.newTask.Set(move_point_task(obj, handle.first,
              obj->GetPoint(handle.first)));
            return TaskResult::CHANGE;
          }
        }
        else if (handle.second == HandleType::EXTENSION_POINT &&
          point_edit_enabled(obj))
        {
          if (info.modifiers.LeftMouse()){
            // Note: This hinges on the MovePointTask not requesting the point for
            // handleIndex + 1 before the command has run.
            int handleIndex = handle.first;
            impl.command.Set(get_appending_insert_command(obj, handle.first,
                info.pos));
            impl.newTask.Set(move_point_task(obj, handleIndex + 1, info.pos));
            return TaskResult::COMMIT_AND_CHANGE;
          }
        }
        return TaskResult::NONE;
      });
  }

  auto hit = info.hitStatus;
  if (hit == Hit::INSIDE || hit == Hit::NEARBY  || hit == Hit::BOUNDARY){
    if (mod_clone_rotate_toggle(info)){
      // Clicked inside object with clone/deselect modifier. Cloning
      // or deselecting depends on drag-distance, see Motion(..).
      if (info.modifiers.RightMouse()){
        if (is_raster_object(info.object)){
          // Handle ctrl+right-mouse on a raster object (set background)
          ObjRaster* rasterObj = as_ObjRaster(info.object);
          Color color(color_at(rasterObj, info.pos));
          impl.command.Set(get_change_raster_background_command(rasterObj, color));
          return TaskResult::COMMIT;
        }
        return TaskResult::NONE;
      }

      impl.SetClickedObject(obj, info.pos, was_selected(true));
      return TaskResult::NONE;
    }
    else if (info.modifiers.RightMouse()){
        bool toggledAlign = toggle_object_aligned_resize(info.object);
        if (toggledAlign){
          impl.fullRefresh = true;
          return TaskResult::DRAW;
        }
        return TaskResult::NONE;
    }
    impl.Reset();
    impl.newTask.Set(move_object_task(obj,
      info.canvas.GetObjectSelection(),
      info.pos - obj->GetTri().P0(),
      MoveMode::MOVE));
    return TaskResult::CHANGE;
  }
  assert(false); // Can't happen
  return TaskResult::NONE;
}

static TaskResult clicked_unselected(IdleSelectionState& impl,
  const PosInfo& info)
{
  impl.SetClickedObject(info.object, info.pos, was_selected(false));
  info.canvas.SelectObject(info.object, deselect_old(!mod_toggle_select(info)));
  impl.fullRefresh = true;
  return TaskResult::DRAW;
}

static TaskResult clone_selected_objects(IdleSelectionState& impl,
  Canvas& canvas,
  const Point& mousePos)
{
  impl.mouseDown = false;
  objects_t objects = canvas.GetObjectSelection();
  canvas.DeselectObjects();
  size_t mainIndex = find_object_index(impl.clicked.object, objects);
  assert(mainIndex != objects.size());
  objects_t clones(clone(objects));
  Object* mainObject = clones[mainIndex];
  impl.Reset();
  impl.settings = get_object_settings(clones);
  impl.newTask.Set(move_object_task(mainObject,
      clones,
      mousePos - mainObject->GetTri().P0(),
      MoveMode::COPY));
  return TaskResult::CHANGE;
}

static TaskResult move_selected_objects(IdleSelectionState& impl,
  const objects_t& objects,
  const Point& mousePos)
{
  Object* mainObject = impl.clicked.object;
  impl.newTask.Set(move_object_task(mainObject,
      objects,
      mousePos - mainObject->GetTri().P0(),
      MoveMode::MOVE));
  return TaskResult::CHANGE;
}

static TaskResult select_object_rectangle(IdleSelectionState& impl,
  const Point& mousePos)
{
  impl.newTask.Set(select_object_rectangle_task(impl.clickPos,
      mousePos,
      impl.settings,
      impl.canvas));
  return TaskResult::CHANGE;
}

class SelectObjectIdle : public StandardTask {
public:
  SelectObjectIdle(Settings& settings, const ActiveCanvas& c)
    : m_impl(settings, c)
  {}

  void Activate() override{
    m_impl.Reset(true);
  }

  TaskResult Char(const KeyInfo& info) override{
    if (info.key.Is(key::enter)){
      // Enter object name on 'enter'
      m_impl.newTask.Set(select_object_set_name(m_impl.canvas));
      return TaskResult::CHANGE;
    }
    return TaskResult::NONE;
  }

  void Draw(FaintDC&, Overlays& overlays, const PosInfo&) override{
    const objects_t& objects = m_impl.canvas->GetObjectSelection();
    for (const Object* obj : objects){
      if (point_edit_disabled(obj)){
        // Skip showing point-editing overlays for this object
        continue;
      }
      for (const Point& pt : obj->GetMovablePoints()){
        overlays.MovablePoint(pt);
      }
      for (const Point& pt : obj->GetExtensionPoints()){
        overlays.ExtensionPoint(pt);
      }
    }
  }

  bool DrawBeforeZoom(Layer) const override{
    return false;
  }

  Command* GetCommand() override{
    return m_impl.command.Take();
  }

  Cursor GetCursor(const PosInfo& info) const override{
    if (info.objSelected){
      if (info.handle.IsSet()){
        return info.handle.Get().Visit(
          [&](const Handle& handle){
            if (mod_rotate(info) && corner_handle(handle)){
              return Cursor::ROTATE;
            }
            if (handle == Handle::P0 || handle == Handle::P3){
              return Cursor::RESIZE_NW;
            }
            else if (handle == Handle::P1 || handle == Handle::P2) {
              return Cursor::RESIZE_NE;
            }
            else if (handle == Handle::P0P2 || handle == Handle::P1P3){
              return Cursor::RESIZE_WE;
            }
            else if (handle == Handle::P0P1 || handle == Handle::P2P3){
              return Cursor::RESIZE_NS;
            }
            else{
              assert(false);
              return Cursor::RESIZE_NS;
            }
          },
          [&](const std::pair<object_handle_t, HandleType>& handle){
            if (handle.second == HandleType::MOVABLE_POINT) {
              return Cursor::MOVE_POINT;
            }
            else if (handle.second == HandleType::EXTENSION_POINT){
              return Cursor::ADD_POINT;
            }
            else{
              assert(false);
              return Cursor::MOVE_POINT;
            }
          });
      }
      else if (info.hitStatus == Hit::INSIDE ||
        info.hitStatus == Hit::NEARBY ||
        info.hitStatus == Hit::BOUNDARY)
      {
        return mod_clone(info) ? Cursor::CLONE : Cursor::MOVE;
      }
    }
    return Cursor::ARROW;
  }

  Task* GetNewTask() override{
    return m_impl.newTask.Take();
  }

  IntRect GetRefreshRect(const RefreshInfo& info) const override{
    if (then_false(m_impl.fullRefresh)){
      // Refresh the entire visible area after selection changes.
      return info.visibleRect;
    }
    // The idle selection for the most time requires no refresh.
    return IntRect(IntPoint(0, 0), IntSize(0, 0));
  }

  TaskResult DoubleClick(const PosInfo& info) override{
    if (!info.objSelected) {
      return TaskResult::NONE;
    }

    bool toggledPoints = toggle_edit_points(info.object);
    if (toggledPoints){
      SelectionChange();
      return TaskResult::SETTING_CHANGED;
    }

    // Retrieve clicked text object or grouped text object, if any
    ObjText* objText = hovered_selected_text(info, SearchMode::include_grouped);
    if (objText != nullptr){
      // Start editing the double-clicked text object
      m_impl.newTask.Set(edit_text_task(objText, m_impl.settings));
      return TaskResult::CHANGE;
    }
    return TaskResult::NONE;
  }

  TaskResult MouseDown(const PosInfo& info) override{
    info.status.SetMainText("");
    if (info.object == nullptr){
      return clicked_nothing(m_impl, info);
    }
    else if (info.objSelected){
      return clicked_selected(m_impl, info);
    }
    return clicked_unselected(m_impl, info);
  }

  TaskResult MouseUp(const PosInfo& info) override{
    if (m_impl.mouseDown && mod_toggle_select(info) &&
      not_drag(m_impl.clickPos, info.pos))
    {
      if (selectable(m_impl.clicked)){
        info.canvas.SelectObject(m_impl.clicked.object, deselect_old(false));
      }
      else if (deselectable(m_impl.clicked)){
        info.canvas.DeselectObject(m_impl.clicked.object);
      }
      m_impl.Reset(true);
      return TaskResult::DRAW;
    }
    m_impl.Reset();
    return TaskResult::NONE;
  }

  TaskResult MouseMove(const PosInfo& info) override{
    info.status.SetText(str(info.pos));

    if (!m_impl.mouseDown || !is_drag(m_impl.clickPos, info.pos)){
      // There's no action for motion when not dragging with mouse held.
      info.status.SetMainText(status_for_object(info));
      return TaskResult::NONE;
    }

    info.status.SetMainText("");
    const bool cloneBtn = info.modifiers.Primary();
    if (deselectable(m_impl.clicked) && cloneBtn){
      return clone_selected_objects(m_impl, info.canvas, info.pos);
    }
    else if (quick_draggable(m_impl.clicked) && !cloneBtn){
      return move_selected_objects(m_impl,
        info.canvas.GetObjectSelection(), info.pos);
    }
    else if (no_object(m_impl.clicked)){
      return select_object_rectangle(m_impl, info.pos);
    }
    return TaskResult::NONE;
  }

  TaskResult Preempt(const PosInfo&) override{
    return TaskResult::NONE;
  }

  void SelectionChange() override{
    m_impl.settings = get_object_settings(m_impl.canvas->
      GetObjectSelection());
  }

  SelectObjectIdle(const SelectObjectIdle&) = delete;
  SelectObjectIdle& operator=(const SelectObjectIdle&) = delete;
private:
  IdleSelectionState m_impl;
  PendingCommand m_command;
};

Task* select_object_idle(Settings& s, const ActiveCanvas& canvas){
  return new SelectObjectIdle(s, canvas);
}

} // namespace
