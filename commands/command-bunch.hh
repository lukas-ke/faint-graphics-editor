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
#include "commands/command-constants.hh"
#include "util/distinct.hh"

namespace faint{
class Command;
class utf8_string;

class category_command_bunch;
using bunch_name = Distinct<utf8_string, category_command_bunch, 0>;

class MergeCondition{
  // Specifies conditions for merging all commands in two
  // CommandBunches, and for appending a single command to
  // an existing CommandBunch.
public:
  virtual ~MergeCondition() = default;

  // True if the command should be appended to the list of commands in
  // the command bunch with this merge condition.
  virtual bool Append(Command*) = 0;

  // True if the command bunch should assume the name of the
  // merged command bunch or appended command.
  virtual bool AssumeName() const = 0;

  // True if all commands should be merged.
  virtual bool Satisfied(MergeCondition*) = 0;
  bool Unsatisfied(MergeCondition*);
};

// Returns a CommandBunch with all the passed in commands, if more
// than one, using the specified bunch_name for the command name. If
// the container only has one command, that command is returned
// unchanged.
Command* perhaps_bunch(CommandType, const bunch_name&,
  const std::vector<Command*>&);

Command* perhaps_bunch(CommandType, const bunch_name&,
  const std::deque<Command*>&);

Command* command_bunch(CommandType, const bunch_name&,
  const std::vector<Command*>&, MergeCondition* c=nullptr);

Command* command_bunch(CommandType, const bunch_name&,
  const std::deque<Command*>&);

Command* command_bunch(CommandType, const bunch_name&, Command*,
  MergeCondition* c=nullptr);

Command* command_bunch(CommandType, const bunch_name&, Command*, Command*,
  MergeCondition* c=nullptr);

} // namespace

#endif
