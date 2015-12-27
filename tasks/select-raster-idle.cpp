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
#include "commands/command.hh"
#include "commands/command-bunch.hh"
#include "commands/set-raster-selection-cmd.hh"
#include "geo/geo-func.hh"
#include "tasks/select-raster-base.hh"
#include "tasks/select-raster-idle.hh"
#include "tasks/select-raster-move.hh"
#include "tasks/select-raster-rectangle.hh"
#include "text/formatting.hh"
#include "tools/tool-actions.hh"
#include "util/image.hh"
#include "util/pos-info.hh"
#include "util/setting-util.hh"
#include "util/tool-util.hh"
#include "util/visit-selection.hh"
#include "util/merge-conditions.hh"
namespace faint{

std::unique_ptr<MergeCondition> append_selection(){
  // Used to append a selection command to a command-bunch.
  //
  // In practice, the use is:
  // When there's a Floating selection and a user draws a new
  // selection rectangle, the Stamp-selection command for the previous
  // floating selection and the command selecting the new rectangle
  // should be combined, so that a single undo unstamps the floating
  // selection and removes the new rectangle, and if followed by redo,
  // the selection is again stamped and the new rectangle selected.
  return append_once_if(is_appendable_raster_selection_command,
    AssumeName::Yes);
}

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

static TaskResult move_selected_content(const PosInfo& info,
  PendingCommand& command,
  PendingTask& newTask,
  Settings& settings,
  Canvas& canvas)
{
  const RasterSelection& selection = info.canvas.GetRasterSelection();

  auto move_floating_content = [&](const auto& s){
    // Helper for copying a sel::Moving or sel::Copying.
    using namespace faint;

    const bool gonnaCopy(gonna_copy(info));
    if (gonnaCopy){
      command.Set(command_bunch(CommandType::HYBRID,
        bunch_name("Clone Selection"),
        stamp_floating_selection_command(s),
        append_selection()));

      set_move_task(newTask, gonnaCopy, info, s.TopLeft(), settings, canvas);
      return TaskResult::COMMIT_AND_CHANGE;
    }
    else{
      set_move_task(newTask, gonnaCopy, info, s.TopLeft(), settings, canvas);
      return TaskResult::CHANGE;
    }
  };

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
      return move_floating_content(s);
    },
    [&](const sel::Copying& s){
      return move_floating_content(s);
    });
}

// Helper for new_selection_rectangle
static void deselect_non_floating(PendingCommand& command, CommandPtr deselect){
  command.Set(command_bunch(CommandType::SELECTION,
    bunch_name("Deselect Raster"),
    std::move(deselect),
    append_selection()));
}

// Helper for new_selection_rectangle. Stamps the selection before
// deselecting.
template<typename SELECTION_TYPE>
void deselect_floating(PendingCommand& command,
  SELECTION_TYPE& s,
  CommandPtr deselect)
{
  command.Set(command_bunch(CommandType::HYBRID,
    bunch_name("Deselect Raster"),
    stamp_floating_selection_command(s),
    std::move(deselect),
    append_selection()));
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
    newTask.Set(
      raster_selection_rectangle_task(
        info.pos,
        settings,
        Old(selection.GetOptions()),
        shouldMerge,
        canvas));
    return TaskResult::CHANGE;
  }

  // Use a task with command merging which doesn't touch the selection
  // settings.
  newTask.Set(raster_selection_rectangle_task(info.pos,
    settings,
    shouldMerge,
    canvas));

  // Create a deselection, and possibly stamping, command. This will
  // be merged later with the command from the selection task (if
  // any).
  auto deselect = set_raster_selection_command(New(SelectionState()),
    Old(selection.GetState()), "Deselect Raster");

  sel::visit(selection,
    [&](const sel::Empty&){
      deselect_non_floating(command, std::move(deselect));
    },
    [&](const sel::Rectangle&){
      deselect_non_floating(command, std::move(deselect));
    },
    [&](const sel::Moving& s){
      deselect_floating(command, s, std::move(deselect));
    },
    [&](const sel::Copying& s){
      deselect_floating(command, s, std::move(deselect));
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
      command.Set(command_bunch(CommandType::HYBRID,
        bunch_name("Clone Selection"),
        stamp_floating_selection_command(s),
        append_selection()));
    },
    [&command](const sel::Copying& s){
      // Lose the floating selection
      command.Set(command_bunch(CommandType::HYBRID,
        bunch_name("Clone Selection"),
        stamp_floating_selection_command(s),
        append_selection()));
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
  Settings& settings,
  ToolActions& actions)
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

  actions.UpdateShownSettings();
  return TaskResult::COMMIT;
}

class RasterSelectionIdle : public RasterSelectionTask {
public:
  RasterSelectionIdle(Settings& s,
    const ActiveCanvas& canvas,
    ToolActions& actions)
    : m_actions(actions),
      m_canvas(canvas),
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

  CommandPtr GetCommand() override{
    return m_command.Take();
  }

  Cursor GetCursor(const PosInfo& info) const override{
    return info.inSelection ? Cursor::MOVE : Cursor::CROSSHAIR;
  }

  TaskPtr GetNewTask() override{
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
          pick_mask_color(info, m_command, m_settings, m_actions) :
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
      else{
        info.status.SetMainText("Click to move the selected content.");
      }
    }
    else{
      info.status.SetMainText("Click to draw a selection rectangle.");
    }
    info.status.SetText(str_floor(info.pos), 0);
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

  ToolActions& m_actions;
  ActiveCanvas m_canvas;
  PendingCommand m_command;
  bool m_fullRefresh;
  PendingTask m_newTask;
  Settings& m_settings;
};

Task* raster_selection_idle_task(Settings& settings,
  const ActiveCanvas& c,
  ToolActions& actions)
{
  return new RasterSelectionIdle(settings, c, actions);
}

} // namespace
