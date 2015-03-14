// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
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
#include "commands/command.hh"
#include "geo/canvas-geo.hh"
#include "geo/geo-func.hh"
#include "gui/canvas-panel-contexts.hh"
#include "text/formatting.hh"
#include "util/command-history.hh"
#include "util/image.hh"
#include "util/image-list.hh"
#include "util/iter.hh"

namespace faint{

static void clear_list(std::deque<OldCommand>& list){
  for (auto& item : list){
    delete item.command;
  }
  list.clear();
}

static utf8_string get_command_name(Command& cmd,
  const Image& targetFrame,
  const ImageList& frames)
{
  bool showFrameIndex = cmd.Type() != CommandType::FRAME &&
    targetFrame.GetId() != frames.Active().GetId();
  if (showFrameIndex){
    const auto index = frames.GetIndex(targetFrame);
    return space_sep(cmd.Name(), bracketed(space_sep("Frame:", str_user(index))));
  }
  return cmd.Name();
}

CommandHistory::CommandHistory()
  : m_openBundle(false)
{}

CommandHistory::~CommandHistory(){
  clear_list(m_redoList);
  clear_list(m_undoList);
}

void CommandHistory::OpenUndoBundle(){
  m_undoList.push_back(OldCommand::OpenGroup());
  m_openBundle = true;
}

void CommandHistory::CloseUndoBundle(const utf8_string& name){
  assert(m_openBundle);
  assert(m_undoList.size() >= 1);

  if (m_undoList[m_undoList.size() - 1].type == UndoType::OPEN_GROUP){
    // The command bundle is empty. This can happen by undoing via
    // Python. - Discard the bundle.
    m_undoList.pop_back();
  }
  else{
    assert(m_undoList.size() >= 2);
    if (m_undoList[m_undoList.size() - 2].type == UndoType::OPEN_GROUP){
      // If the group being closed contains only a single command,
      // remove the the open/close group commands, and just add or merge
      // the command.
      OldCommand cmd = m_undoList.back();
      m_undoList.pop_back();
      m_undoList.pop_back();
      bool merged = !m_undoList.empty() && m_undoList.back().Merge(cmd);
      if (!merged){
        m_undoList.push_back(cmd);
      }
    }
    else{
      // Close the bundle
      if (name.empty()){
        m_undoList.push_back(OldCommand::CloseGroup());
      }
      else{
        m_undoList.push_back(OldCommand::CloseGroup(name));
      }
    }
  }
  m_openBundle = false;
}

  Optional<CommandId> CommandHistory::GetLastModifying() const{
  for (const OldCommand& item : reversed(m_undoList)){
    if (item.type == UndoType::NORMAL_COMMAND){
      Command* cmd = item.command;
      if (cmd->ModifiesState()){
        return option(cmd->GetId());
      }
    }
  }
  return no_option();
}

bool CommandHistory::Bundling() const{
  return m_openBundle;
}

bool CommandHistory::CanRedo() const{
  return !m_redoList.empty();
}

bool CommandHistory::CanUndo() const{
  return !m_undoList.empty();
}

utf8_string CommandHistory::GetRedoName(const ImageList& images) const{
  if (m_redoList.empty()){
    return "";
  }

  const OldCommand& first = m_redoList.front();
  if (first.type == UndoType::OPEN_GROUP){
    for (auto cmd : enumerate(but_first(m_redoList))){
      if (cmd.item.type == UndoType::CLOSE_GROUP){
        return cmd.item.name.IsSet() ?
          cmd.item.name.Get() :
          space_sep("Python Commands", bracketed(str_int(cmd.num)));
      }
    }
    assert(false);
    return "";
  }
  else if (first.type == UndoType::CLOSE_GROUP){
    // This should never be visible to the user.
    return "";
  }
  else{
    Command* cmd = first.command;
    assert(cmd != nullptr);
    return get_command_name(*cmd, *first.targetFrame, images);
  }
}

utf8_string CommandHistory::GetUndoName(const ImageList& images) const{
  if (m_undoList.empty()){
    return "";
  }
  const OldCommand& last = m_undoList.back();
  if (last.type == UndoType::CLOSE_GROUP){
    if (last.name.IsSet()){ // Todo: Visit instead
      return last.name.Get();
    }
    for (auto cmd : enumerate(reversed(but_last(m_undoList)))){
      if (cmd.item.type == UndoType::OPEN_GROUP){
        return space_sep("Python Commands", bracketed(str_int(cmd.num)));
      }
    }
    assert(false);
    return "";
  }
  else if (last.type == UndoType::OPEN_GROUP){
    // This should never be visible to the user.
    return "";
  }
  else {
    Command* cmd = last.command;
    assert(cmd != nullptr);
    return get_command_name(*cmd, *last.targetFrame, images);
  }
}

bool CommandHistory::Undo(TargetableCommandContext& cmdContext,
  const CanvasGeo& geo)
{
  if (m_undoList.empty() || m_undoList.back().type == UndoType::OPEN_GROUP){
    return false;
  }

  OldCommand undone = m_undoList.back();

  if (undone.type != UndoType::NORMAL_COMMAND){
    m_redoList.push_front(undone);
    m_undoList.pop_back();
    // Fixme: Duplication + ineffective
    do{
      undone = m_undoList.back();
      if (undone.type == UndoType::NORMAL_COMMAND){
        CommandType undoType(undone.command->Type());
        cmdContext.SetFrame(undone.targetFrame);

        if (somewhat_reversible(undoType)){
          // Reverse undoable changes
          undone.command->Undo(cmdContext);
        }
        if (!fully_reversible(undoType)){
          // Reset the image and reapply the raster steps of all commands to
          // undo the irreversible changes of the undone command.
          // Fixme: This is extremely wasteful with multiple commands
          cmdContext.RevertFrame();
          for (auto item : but_last(m_undoList)){
            if (item.targetFrame == undone.targetFrame){
              Command& reapplied = *item.command;
              reapplied.DoRaster(cmdContext);
            }
          }
        }
      }
      m_undoList.pop_back();
      m_redoList.push_front(undone);
    } while (undone.type == UndoType::NORMAL_COMMAND);

    return true;
  }

  assert(undone.command != nullptr);
  assert(undone.targetFrame != nullptr);
  CommandType undoType(undone.command->Type());
  Image* activeImage = undone.targetFrame;
  IntSize oldSize(activeImage->GetSize());
  cmdContext.SetFrame(activeImage);
  if (somewhat_reversible(undoType)){
    // Reverse undoable changes
    undone.command->Undo(cmdContext);
  }
  if (!fully_reversible(undoType)){
    // Reset the image and reapply the raster steps of all commands to
    // undo the irreversible changes of the undone command.
    cmdContext.RevertFrame();
    for (auto item : but_last(m_undoList)){
      if (item.targetFrame == activeImage){
        Command* reapplied = item.command;
        reapplied->DoRaster(cmdContext);
      }
    }
    if (oldSize != activeImage->GetSize()){
      Point pos(geo.pos.x, geo.pos.y);
      coord zoom = geo.zoom.GetScaleFactor();
      pos = (undone.command->UndoTranslate(pos / zoom) * zoom);
      // AdjustScrollbars(floored(pos)); // Fixme
    }
  }

  m_undoList.pop_back();
  m_redoList.push_front(undone);
  return true;
}

void CommandHistory::Redo(TargetableCommandContext& cmdContext,
  const CanvasGeo& geo,
  ImageList& images)
{
  if (m_redoList.empty()){
    return;
  }
  OldCommand redone = m_redoList.front();
  m_redoList.pop_front();

  if (redone.type != UndoType::NORMAL_COMMAND){
    // Redo the entire command list
    m_undoList.push_back(redone);
    do{
      redone = m_redoList.front();
      m_redoList.pop_front();
      if (redone.type == UndoType::NORMAL_COMMAND){
        assert(redone.targetFrame != nullptr);
        Apply(redone.command, clear_redo(false), redone.targetFrame,
          images, cmdContext, geo);
      }
    } while (redone.type == UndoType::NORMAL_COMMAND);
    m_undoList.push_back(redone);
  }
  else{
    assert(redone.targetFrame != nullptr);
    Apply(redone.command,
      clear_redo(false),
      redone.targetFrame,
      images,
      cmdContext,
      geo);
  }
}

Optional<IntPoint> CommandHistory::Apply(Command* cmd,
  const clear_redo& clearRedo,
  Image* activeImage,
  ImageList& images,
  TargetableCommandContext& commandContext,
  const CanvasGeo& geo)
{
  assert(cmd != nullptr);
  const bool targetCurrentFrame = (activeImage == &images.Active());
  commandContext.SetFrame(activeImage);

  if (affects_raster(cmd)){
    if (!activeImage->HasStoredOriginal()){
      // Store the bitmap data (for undo) on the first change.
      activeImage->StoreAsOriginal();
    }
    if (!activeImage->GetBackground().Get<Bitmap>()){
      activeImage->ConvertColorSpanToBitmap();
    }
  }

  IntSize oldSize(activeImage->GetSize());
  Optional<IntPoint> offset;
  cmd->Do(commandContext);
  if (oldSize != activeImage->GetSize()){
    if (targetCurrentFrame){
      Point pos(geo.pos.x, geo.pos.y); // Fixme: geo should have an IntPoint
      coord zoom = geo.zoom.GetScaleFactor();
      offset.Set(floored(cmd->Translate(pos / zoom) * zoom));
    }

    // Clip the current selection to the new image size
    activeImage->GetRasterSelection().Clip(image_rect(*activeImage)); // Fixme: Move selection to image, fixme: Undo?
  }

  if (clearRedo.Get()){
    clear_list(m_redoList);
  }

  if (Bundling()){
    m_undoList.push_back(OldCommand(cmd, activeImage));
  }
  else{
    OldCommand mappedCmd(cmd, activeImage);
    bool merged = !m_undoList.empty() && m_undoList.back().Merge(mappedCmd);
    if (merged){
      cmd = nullptr;
    }
    else{
      m_undoList.push_back(OldCommand(cmd, activeImage));
    }
  }
  return offset;
}

bool CommandHistory::ApplyDWIM(ImageList& images,
  TargetableCommandContext& ctx,
  const CanvasGeo& geo)
{
  if (m_undoList.empty() || !m_undoList.back().command->HasDWIM()){
    return false;
  }
  Command* dwim = m_undoList.back().command->GetDWIM();
  Undo(ctx, geo);
  Apply(dwim, clear_redo(true), &images.Active(), images, ctx, geo);
  return true;
}

} // namespace
