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

#ifndef FAINT_ADD_OBJECT_CMD_HH
#define FAINT_ADD_OBJECT_CMD_HH
#include "commands/command-constants.hh"
#include "commands/command-ptr.hh"
#include "text/utf8-string.hh"
#include "util/optional.hh"

namespace faint{

class Command;
class Object;

CommandPtr add_object_command(Object*,
  const select_added&,
  const utf8_string& name="Add");

CommandPtr insert_object_command(Object*,
  const select_added&,
  int z,
  const utf8_string& name="Add");

} // namespace

#endif
