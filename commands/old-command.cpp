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

#include "commands/command.hh"
#include "commands/old-command.hh"

namespace faint{

OldCommand::OldCommand(UndoType type)
  : command(nullptr),
    targetFrame(nullptr),
    type(type)
{
  assert(type != UndoType::NORMAL_COMMAND);
}

OldCommand::OldCommand(Command* command, Image* targetFrame)
  : command(command),
    targetFrame(targetFrame),
    type(UndoType::NORMAL_COMMAND)
{
  assert(command != nullptr);
  assert(targetFrame != nullptr);
}

OldCommand OldCommand::OpenGroup(){
  return OldCommand(UndoType::OPEN_GROUP);
}

OldCommand OldCommand::CloseGroup(){
  return OldCommand(UndoType::CLOSE_GROUP);
}

OldCommand OldCommand::CloseGroup(const utf8_string& name){
  OldCommand cmd(UndoType::CLOSE_GROUP);
  cmd.name.Set(name);
  return cmd;
}

bool OldCommand::Merge(OldCommand& candidate){
  return type == UndoType::NORMAL_COMMAND &&
    candidate.type == UndoType::NORMAL_COMMAND &&
    command->Merge(candidate.command,
    targetFrame == candidate.targetFrame);
}

} // namespace
