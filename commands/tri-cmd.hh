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

#ifndef FAINT_TRI_CMD_HH
#define FAINT_TRI_CMD_HH
#include "commands/command.hh"
#include "geo/tri.hh"
#include "text/utf8-string.hh"
#include "util/distinct.hh"

namespace faint{

class Object;

using NewTri = Order<Tri>::New;
using OldTri = Order<Tri>::Old;

enum class MergeMode {
  // Whether a TriCommand wants to merge with consecutive TriCommands
  // targetting the same object.
  SOCIABLE, SOLITARY};

CommandPtr tri_command(Object*,
  const NewTri&,
  const OldTri&,
  const utf8_string& name="Adjust",
  MergeMode=MergeMode::SOLITARY);

std::unique_ptr<class MergeCondition> append_tri_commands();

} // namespace

#endif
