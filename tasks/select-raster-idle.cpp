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

#include "app/active-canvas.hh"
#include "app/get-app-context.hh"
#include "commands/command.hh"
#include "commands/command-bunch.hh"
#include "commands/set-raster-selection-cmd.hh"
#include "geo/geo-func.hh"
#include "tasks/select-raster-base.hh"
#include "tasks/select-raster-idle.hh"
#include "tasks/select-raster-move.hh"
#include "tasks/select-raster-rectangle.hh"
#include "text/formatting.hh"
#include "util/image.hh"
#include "util/pos-info.hh"
#include "util/setting-util.hh"
#include "util/tool-util.hh"
#include "util/visit-selection.hh"

namespace faint{

class AppendSelection : public MergeCondition{
  // Used to add the selection to this command-bunch
public:
  AppendSelection()
    : m_appended(false)
  {}

  bool Satisfied(MergeCondition*) override{
    // AppendSelection is only used for appending,
    // not merging CommandBunches.
    return false;
  }

  bool Append(Command* cmd) override{
    if (m_appended){
      // Only append a single command
      return false;
    }
    m_appended = true;

    return is_appendable_raster_selection_command(cmd);
  }
  bool AssumeName() const override{
    // The rectangle selection command name should be used.
    return true;
  }
private:
  bool m_appended;
};

static TaskResult deselect(const RasterSelection& selection,
  PendingCommand& command)
{
  command.Set(set_raster_selection_command(New(SelectionState()),
      Old(selection.GetState()), "Deselect"));
  return TaskResult::COMMIT;
}

static TaskResult disable_masking(const RasterSelection& selection,
  PendingCommand& command)
{
  SelectionOptions options(selection.GetOptions());
  options.mask = false;
  command.Set(set_selection_options_command(New(options),
      Old(selection.GetOptions())));
  return TaskResult::COMMIT;
}

static void set_move_task(PendingTask& newTask, bool gonnaCopy,
  const PosInfo& info,
  const IntPoint& topLeft,
  Settings& settings,
  Canvas& canvas)
{
  IntPoint offset = floored(info.pos) - topLeft;
  newTask.Set(raster_selection_move_task(offset, topLeft,
    copy_selected(gonnaCopy),
    true, // Float selected
    settings, canvas));
}

static bool gonna_copy(const PosInfo& info){
  return info.modifiers.Primary();
}

// Helper for move_selected_content, SELECTION_TYPE is either a
// sel::Moving or sel::Copying.
// C++14: Replace with generic lambda and pass to visit or add
// dispatch for sel::Floating
template<typename SELECTION_TYPE>
TaskResult move_floating_content(const PosInfo& info,
  PendingCommand& command,
  PendingTask& newTask,
  Settings& settings,
  const SELECTION_TYPE& s,
  Canvas& canvas)
{
  const bool gonnaCopy(gonna_copy(info));
  if (gonnaCopy){
    Command* stamp = stamp_floating_selection_command(s);
    bunch_name("Clone Selection");
    command.Set(command_bunch(CommandType::HYBRID,
        bunch_name("Clone Selection"),
        stamp, new AppendSelection()));
  }
  set_move_task(newTask, gonnaCopy, info, s.TopLeft(), settings, canvas);
  return gonnaCopy ? TaskResult::COMMIT_AND_CHANGE : TaskResult::CHANGE;
}

static TaskResult move_selected_content(const PosInfo& info,
  PendingCommand& command,
  PendingTask& newTask,
  Settings& settings,
  Canvas& canvas)
{
  const RasterSelection& selection = info.canvas.GetRasterSelection();

  return sel::visit(selection,
    [](const sel::Empty&){
      return TaskResult::DRAW;
    },
    [&](const sel::Rectangle& s){
      set_move_task(newTask, gonna_copy(info), info, s.TopLeft(), settings,
        canvas);
      return TaskResult::CHANGE;
    },
    [&](const sel::Moving& s){
      return move_floating_content(info, command, newTask, settings, s, canvas);
    },
    [&](const sel::Copying& s){
      return move_floating_content(info, command, newTask, settings, s, canvas);
    });
}

// Helper for new_selection_rectangle
static void deselect_non_floating(PendingCommand& command, Command* deselect){
  command.Set(command_bunch(CommandType::SELECTION,
    bunch_name("Deselect Raster"),
    deselect,
    new AppendSelection()));
}

// Helper for new_selection_rectangle. Stamps the selection before
// deselecting.
template<typename SELECTION_TYPE>
void deselect_floating(PendingCommand& command,
  SELECTION_TYPE& s,
  Command* deselect)
{
  command.Set(command_bunch(CommandType::HYBRID,
    bunch_name("Deselect Raster"),
    stamp_floating_selection_command(s),
    deselect,
    new AppendSelection()));
}

static TaskResult new_selection_rectangle(const PosInfo& info,
  PendingCommand& command,
  PendingTask& newTask,
  Settings& settings,
  const ActiveCanvas& canvas)
{
  const RasterSelection& selection(info.canvas.GetRasterSelection());
  bool shouldMerge = selection.Exists();
  if (!shouldMerge){
    // Use a task without command merging (since no deselection or
    // stamping was required). Also update the selection settings
    newTask.Set(raster_selection_rectangle_task(info.pos, settings,
        Old(selection.GetOptions()), shouldMerge, canvas));
    return TaskResult::CHANGE;
  }

  // Use a task that with command merging which doesn't touch touch
  // the selection settings.
  newTask.Set(raster_selection_rectangle_task(info.pos, settings, shouldMerge,
    canvas));

  // Create a deselection, and possibly stamping, command. This will
  // be merged later with the command from the selection task (if
  // any).
  Command* deselect = set_raster_selection_command(New(SelectionState()),
    Old(selection.GetState()), "Deselect Raster");

  sel::visit(selection,
    [&](const sel::Empty&){
      deselect_non_floating(command, deselect);
    },
    [&](const sel::Rectangle&){
      deselect_non_floating(command, deselect);
    },
    [&](const sel::Moving& s){
      deselect_floating(command, s, deselect);
    },
    [&](const sel::Copying& s){
      deselect_floating(command, s, deselect);
    });
  return TaskResult::COMMIT_AND_CHANGE;
}

static TaskResult move_selection_rect(const PosInfo& info,
  PendingCommand& command,
  PendingTask& newTask,
  Settings& settings,
  Canvas& canvas)
{
  const RasterSelection& selection = info.canvas.GetRasterSelection();
  const IntPoint pos(floored(info.pos));
  IntPoint offset = pos - selection.TopLeft();

  sel::visit(selection,
    [](const sel::Empty&){
      // Nothing to do
    },
    [](const sel::Rectangle&){
      // Nothing to do
    },
    [&command](const sel::Moving& s){
      // Lose the floating selection
      Command* stamp = stamp_floating_selection_command(s);
      command.Set(command_bunch(CommandType::HYBRID,
        bunch_name("Clone Selection"),
        stamp, new AppendSelection()));
    },
    [&command](const sel::Copying& s){
      // Lose the floating selection
      Command* stamp = stamp_floating_selection_command(s);
      command.Set(command_bunch(CommandType::HYBRID,
        bunch_name("Clone Selection"),
        stamp, new AppendSelection()));
    });

  newTask.Set(raster_selection_move_task(offset, selection.TopLeft(),
    copy_selected(false),
    false, // Don't float selected
    settings, canvas));
  return command.Valid() ? TaskResult::COMMIT_AND_CHANGE : TaskResult::CHANGE;
}

static Color selection_or_image_color(const RasterSelection& selection,
  const Image& image,
  IntPoint pos)
{
  return sel::visit(selection,
    [&image, &pos](const sel::Empty&){
      return get_color(image.GetBackground(), pos);
    },
    [&image, &pos](const sel::Rectangle&){
      return get_color(image.GetBackground(), pos);
    },
    [&pos](const sel::Floating& s){
      IntPoint offset = pos - s.TopLeft();
      auto& bmp(s.GetBitmap());
      assert(point_in_bitmap(bmp, offset));
      return get_color(bmp, offset);
    });
}


static TaskResult pick_mask_color(const PosInfo& info,
  PendingCommand& command,
  Settings& settings)
{
  assert(info.inSelection);
  const RasterSelection& selection(info.canvas.GetRasterSelection());
  const IntPoint pos(floored(info.pos));

  SelectionOptions newOptions(true,
    Paint(selection_or_image_color(selection, info.canvas.GetImage(), pos)),
    settings.Get(ts_AlphaBlending));

  settings.Set(ts_Bg, newOptions.bg);
  settings.Set(ts_BackgroundStyle, newOptions.mask ? BackgroundStyle::MASKED :
    BackgroundStyle::SOLID);
  command.Set(set_selection_options_command(New(newOptions),
      Old(selection.GetOptions())));

  // This update is still required. Maybe add to ActiveCanvas.
  get_app_context().UpdateShownSettings();
  return TaskResult::COMMIT;
}

class RasterSelectionIdle : public RasterSelectionTask {
public:
  RasterSelectionIdle(Settings& s, const ActiveCanvas& canvas)
    : m_canvas(canvas),
      m_fullRefresh(false),
      m_settings(s)
  {}

  void Activate() override{
    // Use full refresh to clear the single selection pixel left when
    // deselecting with SelectRectangle.
    m_fullRefresh = true;
  }

  void Draw(FaintDC&, Overlays&, const PosInfo&) override{
    m_fullRefresh = false;
  }

  bool DrawBeforeZoom(Layer) const override{
    return false;
  }

  Command* GetCommand() override{
    return m_command.Take();
  }

  Cursor GetCursor(const PosInfo& info) const override{
    return info.inSelection ? Cursor::MOVE : Cursor::CROSSHAIR;
  }

  Task* GetNewTask() override{
    return m_newTask.Take();
  }

  IntRect GetRefreshRect(const RefreshInfo& info) const override{
    return m_fullRefresh ? info.visibleRect : IntRect::EmptyRect();
  }

  TaskResult MouseDown(const PosInfo& info) override{
    if (info.modifiers.RightMouse()){
      const RasterSelection& selection = info.canvas.GetRasterSelection();
      if (!selection.Exists()){
        // Right click without selection means nothing.
        return TaskResult::NONE;
      }
      if (info.modifiers.Primary()){
        return info.inSelection ?
          pick_mask_color(info, m_command, m_settings) :
          disable_masking(info.canvas.GetRasterSelection(), m_command);
      }
      else if (info.inSelection){
        return move_selection_rect(info, m_command, m_newTask, m_settings,
          *m_canvas);
      }
      else{
        return deselect(info.canvas.GetRasterSelection(), m_command);
      }
    }
    else if (info.inSelection){
      return move_selected_content(info, m_command, m_newTask, m_settings,
        *m_canvas);
    }
    else{
      return new_selection_rectangle(info, m_command, m_newTask, m_settings,
        m_canvas);
    }
  }

  TaskResult MouseUp(const PosInfo&) override{
    return TaskResult::NONE;
  }

  TaskResult MouseMove(const PosInfo& info) override{
    if (info.inSelection){
      const RasterSelection& selection(info.canvas.GetRasterSelection());
      if (selection.Floating()){
        info.status.SetMainText("Click to move the floating selection.");
      }
      else {
        info.status.SetMainText("Click to move the selected content.");
      }
    }
    else {
      info.status.SetMainText("Click to draw a selection rectangle.");
    }
    info.status.SetText(str(info.pos), 0);
    return TaskResult::NONE;
  }

  TaskResult Preempt(const PosInfo&) override{
    return TaskResult::NONE;
  }

  void SelectionChange() override{
    m_settings = get_selection_settings(m_canvas->GetRasterSelection());
  }

  RasterSelectionIdle& operator=(const RasterSelectionIdle&) = delete;
private:
  Canvas& GetCanvas() const override{
    return *m_canvas;
  }

  const Settings& GetSettings() override{
    return m_settings;
  }

  ActiveCanvas m_canvas;
  PendingCommand m_command;
  bool m_fullRefresh;
  PendingTask m_newTask;
  Settings& m_settings;
};

Task* raster_selection_idle_task(Settings& settings, const ActiveCanvas& c){
  return new RasterSelectionIdle(settings, c);
}

} // namespace
