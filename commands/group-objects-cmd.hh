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

#ifndef FAINT_GROUP_OBJECTS_CMD_HH
#define FAINT_GROUP_OBJECTS_CMD_HH
#include <utility>
#include "util/objects.hh"
#include "commands/command-constants.hh"

namespace faint{

class Command;

using cmd_and_group_t = std::pair<Command*, Object*>;

// Group the specified objects into a new object. The list of objects
// must not be empty. Returns the command and the group.
cmd_and_group_t group_objects_command(const objects_t&, const select_added&);

// Ungroup the specified objects (a list of groups). The list of
// objects must not be empty and each objects must contain sub-objects
// (GetObjectCount() > 0)
Command* ungroup_objects_command(const objects_t&, const select_added&);

} // namespace

#endif
