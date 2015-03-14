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

#include "commands/tri-cmd.hh"
#include "objects/object.hh"

namespace faint{

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

bool TriCommand::Merge(Command* cmd, bool sameFrame){
  if (!m_mergable || !sameFrame){
    return false;
  }

  TriCommand* candidate = dynamic_cast<TriCommand*>(cmd);
  if (candidate == nullptr || !candidate->m_mergable){
    return false;
  }
  if (m_object != candidate->m_object){
    return false;
  }

  m_new = candidate->m_new;
  delete candidate;
  return true;
}

utf8_string TriCommand::Name() const{
  return m_name + " " + m_object->GetType();
}

void TriCommand::Undo(CommandContext&){
  m_object->SetTri(m_old);
}

} // namespace
