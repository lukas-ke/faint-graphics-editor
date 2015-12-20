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

#ifndef FAINT_COMMAND_BUNCH_HH
#define FAINT_COMMAND_BUNCH_HH
#include <deque>
#include <vector>
#include <memory>
#include "commands/command-constants.hh"
#include "commands/command-ptr.hh"
#include "commands/merge-condition.hh"
#include "util/distinct.hh"

namespace faint{
class Command;
class utf8_string;

class category_command_bunch;
using bunch_name = Distinct<utf8_string, category_command_bunch, 0>;

// Returns a CommandBunch with all the passed in commands, if more
// than one, using the specified bunch_name for the command name. If
// the container only has one command, that command is returned
// unchanged.
CommandPtr perhaps_bunch(CommandType,
  const bunch_name&,
  commands_t);

CommandPtr perhaps_bunch(CommandType,
  const bunch_name&,
  std::deque<CommandPtr>);

CommandPtr perhaps_bunch(CommandType,
  const bunch_name&,
  commands_t);

CommandPtr command_bunch(CommandType,
  const bunch_name&,
  commands_t,
  MergeConditionPtr c=nullptr);

CommandPtr command_bunch(CommandType,
  const bunch_name&,
  std::deque<CommandPtr>);

CommandPtr command_bunch(CommandType,
  const bunch_name&,
  CommandPtr,
  MergeConditionPtr c=nullptr);

CommandPtr command_bunch(CommandType,
  const bunch_name&,
  CommandPtr,
  CommandPtr,
  MergeConditionPtr c=nullptr);

} // namespace

#endif
