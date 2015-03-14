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
  SOCIABLE, SOLITARY };

class TriCommand : public Command {
  // Command which changes the boundary of an object.
  // Allows specifying the name to describe the change more precisely
  // (e.g. rotation, translation, scaling).
public:
  TriCommand(Object*,
    const NewTri&,
    const OldTri&,
    const utf8_string& name="Adjust",
    MergeMode=MergeMode::SOLITARY);

  void Do(CommandContext&) override;
  bool Merge(Command*, bool) override;
  utf8_string Name() const override;
  void Undo(CommandContext&) override;

private:
  TriCommand& operator=(const TriCommand&);
  Object* m_object;
  Tri m_new;
  const Tri m_old;
  utf8_string m_name;
  bool m_mergable;
};

} // namespace

#endif
