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

#include "util/append-command-type.hh"
#include "commands/command.hh"
#include "commands/tri-cmd.hh"
#include "objects/object.hh"

namespace faint{

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
  bool Merge(CommandPtr&, bool) override;
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


TriCommand::TriCommand(Object* object,
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

void TriCommand::Do(CommandContext&){
  m_object->SetTri(m_new);
}

bool TriCommand::Merge(CommandPtr& cmd, bool sameFrame){
  if (!m_mergable || !sameFrame){
    return false;
  }

  auto get_command_tri = [&]() -> Optional<Tri>{
    TriCommand* candidate = dynamic_cast<TriCommand*>(cmd.get());
    if (candidate == nullptr){
      return {};
    }
    else if (!candidate->m_mergable){
      return {};
    }
    else if (m_object != candidate->m_object){
      return {};
    }
    else{
      return option(candidate->m_new);
    }
  };

  return get_command_tri().Visit(
    [&](const Tri& mergedTri){
      CommandPtr release(std::move(cmd));
      m_new = mergedTri;
      return true;
    },
    [&](){
      return false;
    });
}

utf8_string TriCommand::Name() const{
  return m_name + " " + m_object->GetType();
}

void TriCommand::Undo(CommandContext&){
  m_object->SetTri(m_old);
}

CommandPtr tri_command(Object* obj,
  const NewTri& newTri,
  const OldTri& oldTri,
  const utf8_string& name,
  MergeMode mergeMode)
{
  return std::make_unique<TriCommand>(obj, newTri, oldTri, name, mergeMode);
}

MergeCondition* append_tri_commands(){
  return new AppendCommandType<TriCommand>();
}

} // namespace
