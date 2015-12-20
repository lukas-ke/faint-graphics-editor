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

#include "commands/command.hh"
#include "commands/tri-cmd.hh"
#include "objects/object.hh"
#include "util/append-command-type.hh"

namespace faint{

class TriCommand : public Command {
  // Command which changes the boundary of an object.
  // Allows specifying the name to describe the change more precisely
  // (e.g. rotation, translation, scaling).
public:
  TriCommand(Object* object,
    const NewTri& newTri,
    const OldTri& oldTri,
    const utf8_string& name,
    MergeMode mergeMode)
    : Command(CommandType::OBJECT),
      m_object(object),
      m_new(newTri.Get()),
      m_old(oldTri.Get()),
      m_name(name),
      m_mergable(mergeMode == MergeMode::SOCIABLE)
  {}

  void Do(CommandContext&) override{
    m_object->SetTri(m_new);
  }

  bool ShouldMerge(const Command& cmd, bool sameFrame) const override{
    if (!m_mergable || !sameFrame){
      return false;
    }

    return if_type<const TriCommand>(cmd,
      [&](const TriCommand& cmd){
        return cmd.ShouldMergeFor(m_object);
      },
      false_f);
  }

  void Merge(CommandPtr cmd) override{
    DoMerge(unique_ptr_cast<TriCommand>(std::move(cmd)));
  }

  utf8_string Name() const override{
    return m_name + " " + m_object->GetType();
  }

  void Undo(CommandContext&) override{
    m_object->SetTri(m_old);
  }

private:
  void DoMerge(std::unique_ptr<TriCommand> cmd){
    assert(m_mergable);
    assert(cmd->m_mergable);
    assert(m_object == cmd->m_object);
    m_new = cmd->m_new;
  }
  bool ShouldMergeFor(Object* object) const{
    return m_mergable && object == m_object;
  }

  TriCommand& operator=(const TriCommand&);
  Object* m_object;
  Tri m_new;
  const Tri m_old;
  utf8_string m_name;
  bool m_mergable;
};

CommandPtr tri_command(Object* obj,
  const NewTri& newTri,
  const OldTri& oldTri,
  const utf8_string& name,
  MergeMode mergeMode)
{
  return std::make_unique<TriCommand>(obj, newTri, oldTri, name, mergeMode);
}

std::unique_ptr<MergeCondition> append_tri_commands(){
  return append_once_if_type<TriCommand>();
}

} // namespace
