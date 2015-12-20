// -*- coding: us-ascii-unix -*-
// Copyright 2013 Lukas Kemmer
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

#include <memory>
#include "commands/command.hh"
#include "commands/set-raster-selection-cmd.hh"
#include "geo/geo-func.hh"
#include "geo/int-point.hh"
#include "geo/tri.hh"
#include "rendering/faint-dc.hh"
#include "text/utf8-string.hh"
#include "util/optional.hh"
#include "util/setting-util.hh"

namespace faint{

class SetSelectionOptionsCommand : public Command {
public:
  SetSelectionOptionsCommand(const NewSelectionOptions& newOptions,
    const OldSelectionOptions& oldOptions)
    : Command(CommandType::SELECTION),
      m_newOptions(newOptions.Get()),
      m_oldOptions(oldOptions.Get())
  {}

  void Do(CommandContext& ctx) override{
    ctx.SetRasterSelectionOptions(m_newOptions);
  }

  utf8_string Name() const override{
    return "Change Selection Settings";
  }

  void Undo(CommandContext& ctx) override{
    ctx.SetRasterSelectionOptions(m_oldOptions);
  }
private:
  SelectionOptions m_newOptions;
  SelectionOptions m_oldOptions;
};

class SetRasterSelectionCommand : public Command{
public:
  SetRasterSelectionCommand(const NewSelectionState& newState,
    const OldSelectionState& oldState,
    const utf8_string& name,
    bool appendCommand)
    : Command(CommandType::SELECTION),
      m_appendCommand(appendCommand),
      m_name(name),
      m_newState(newState.Get()),
      m_oldState(oldState.Get())
  {}

  SetRasterSelectionCommand(const NewSelectionState& newState,
    const Alternative<SelectionState> altNewState,
    const OldSelectionState& oldState,
    const utf8_string& name,
    bool appendCommand)
    : Command(CommandType::SELECTION),
      m_altNewState(altNewState.Get()),
      m_appendCommand(appendCommand),
      m_name(name),
      m_newState(newState.Get()),
      m_oldState(oldState.Get())
  {}

  CommandPtr GetDWIM() override{
    return m_altNewState.Visit(
      [&](const SelectionState& altNewState) -> CommandPtr{
        // Fixme: Clone options command
        return std::make_unique<SetRasterSelectionCommand>(
          New(altNewState),
          alternate(m_newState),
          Old(m_oldState),
          m_name,
          false);
      },
      []() -> CommandPtr{
        assert(false); // Should not be called when not HasDWIM().
        return nullptr;
      });
  }

  void Do(CommandContext& ctx) override{
    if (m_optionsCommand != nullptr){
      m_optionsCommand->Do(ctx);
    }
    ctx.SetRasterSelection(m_newState);
  }

  bool HasDWIM() const override{
    return m_altNewState.IsSet();
  }

  bool ModifiesState() const override{
    // Moving a floating selection or changing to/from floating state
    // affects the image content, merely selecting a region does not.
    return m_newState.Floating() || m_oldState.Floating();
  }

  utf8_string Name() const override{
    return m_name;
  }

  void Undo(CommandContext& ctx) override{
    ctx.SetRasterSelection(m_oldState);
    if (m_optionsCommand != nullptr){
      m_optionsCommand->Undo(ctx);
    }
  }

  bool ShouldAppend() const {
    // Fixme: Remove?
    // ..Presumably this refers to preferring a CommandBunch with
    // ..AppendSelection instead. Check if this is still used
    return m_appendCommand;
  }

  void SetOptionsCommand(SetSelectionOptionsCommand* cmd) {
    // Fixme: Remove?
    // ..Presumably this refers to using a command bunch instead
    // ..of appending the selections to the selection.
    m_optionsCommand.reset(cmd);
  }

  SetRasterSelectionCommand& operator=(const SetRasterSelectionCommand&) = delete;

private:
  Optional<SelectionState> m_altNewState;
  bool m_appendCommand;
  const utf8_string m_name;
  SelectionState m_newState;
  SelectionState m_oldState;
  std::unique_ptr<SetSelectionOptionsCommand> m_optionsCommand;
};

class MoveRasterSelectionCommand : public Command {
public:
  MoveRasterSelectionCommand(const IntPoint& newPos, const IntPoint& oldPos)
    : Command(CommandType::SELECTION),
      m_newPos(newPos),
      m_oldPos(oldPos)
  {}

  bool ShouldMerge(const Command& cmd, bool sameFrame) const override{
    if (!sameFrame){
      return false;
    }
    return dynamic_cast<const MoveRasterSelectionCommand*>(&cmd) != nullptr;
  }

  void Merge(CommandPtr cmd) override{
    // Merge with other, consecutive, move-raster-selection commands
    // by using their position

    const auto* candidate =
      dynamic_cast<const MoveRasterSelectionCommand*>(cmd.get());
    assert(candidate != nullptr);
    m_newPos = candidate->m_newPos;
  }

  void Do(CommandContext& ctx) override{
    ctx.MoveRasterSelection(m_newPos);
  }

  bool ModifiesState() const override{
    return true;
  }

  utf8_string Name() const override{
    return "Move Selected Content";
  }

  void Undo(CommandContext& ctx) override{
    ctx.MoveRasterSelection(m_oldPos);
  }

private:
  IntPoint m_newPos;
  IntPoint m_oldPos;
};

CommandPtr move_raster_selection_command(const IntPoint& newPos,
  const IntPoint& oldPos)
{
  return std::make_unique<MoveRasterSelectionCommand>(newPos, oldPos);
}

class StampFloatingSelectionCommand : public Command {
public:
  StampFloatingSelectionCommand(const Bitmap& bmp,
    const IntRect& r,
    const Optional<IntRect>& oldRect,
    const SelectionOptions& options)
    : Command(CommandType::RASTER),
      m_bitmap(bmp),
      m_oldRect(oldRect),
      m_rect(r),
      m_settings(bitmap_mask_settings(options.mask, options.bg, options.alpha)),
      m_name("Stamp selection")
  {}

  void Do(CommandContext& context) override{
    FaintDC& dc(context.GetDC());
    if (m_oldRect.IsSet()){
      dc.Rectangle(tri_from_rect(floated(m_oldRect.Get())),
        eraser_rectangle_settings(m_settings.Get(ts_Bg)));
    }
    dc.Blit(m_bitmap, floated(m_rect.TopLeft()), m_settings);
  }

  utf8_string Name() const override{
    return m_name;
  }
private:
  Bitmap m_bitmap;
  Optional<IntRect> m_oldRect;
  IntRect m_rect;
  Settings m_settings;
  utf8_string m_name;
};

std::unique_ptr<Command> stamp_floating_selection_command(
  const sel::Copying& copying)
{
  return std::make_unique<StampFloatingSelectionCommand>(copying.GetBitmap(),
    copying.Rect(),
    no_option(),
    copying.GetOptions());
}

std::unique_ptr<Command> stamp_floating_selection_command(
  const sel::Moving& moving)
{
  return std::make_unique<StampFloatingSelectionCommand>(moving.GetBitmap(),
    moving.Rect(),
    option(moving.OldRect()),
    moving.GetOptions());
}

CommandPtr set_selection_options_command(const NewSelectionOptions& newOptions,
  const OldSelectionOptions& oldOptions)
{
  return std::make_unique<SetSelectionOptionsCommand>(newOptions, oldOptions);
}

std::unique_ptr<Command> set_raster_selection_command(
  const NewSelectionState& newState,
  const OldSelectionState& oldState,
  const utf8_string& name,
  bool appendCommand)
{
  return std::make_unique<SetRasterSelectionCommand>(
    newState,
    oldState,
    name,
    appendCommand);
}

std::unique_ptr<Command> set_raster_selection_command(
  const NewSelectionState& newState,
  const Alternative<SelectionState>& altNewState,
  const OldSelectionState& oldState,
  const utf8_string& name,
  bool appendCommand)
{
  return std::make_unique<SetRasterSelectionCommand>(newState,
    altNewState,
    oldState,
    name,
    appendCommand);
}

std::unique_ptr<Command> set_raster_selection_command(
  const NewSelectionState& newState,
  const OldSelectionState& oldState,
  const utf8_string& name,
  bool appendCommand,
  const NewSelectionOptions& newOptions,
  const OldSelectionOptions& oldOptions)
{
  auto cmd = std::make_unique<SetRasterSelectionCommand>(newState,
    oldState,
    name,
    appendCommand);

  cmd->SetOptionsCommand(new SetSelectionOptionsCommand(newOptions,
    oldOptions));
  return cmd;
}

bool is_appendable_raster_selection_command(const Command& cmd){
  const auto* candidate = dynamic_cast<const SetRasterSelectionCommand*>(&cmd);
  return candidate != nullptr && candidate->ShouldAppend();
}

bool is_move_raster_selection_command(const Command& cmd){
  return dynamic_cast<const MoveRasterSelectionCommand*>(&cmd) != nullptr;
}


} // namespace
