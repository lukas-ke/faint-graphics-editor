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

#include "commands/add-object-cmd.hh"
#include "commands/command.hh"
#include "objects/object.hh"
#include "util/convenience.hh"

namespace faint{

class AddObjectCommand : public Command {
public:
  AddObjectCommand(Object* object,
    const select_added& select,
    const utf8_string& name,
    int z)
    : Command(CommandType::OBJECT),
      m_object(object),
      m_select(select.Get()),
      m_name(name),
      m_z(z)
  {}

  ~AddObjectCommand(){
    delete m_object;
  }

  void Do(CommandContext& context) override{
    if (m_z == -1){
      context.Add(m_object, select_added(then_false(m_select)),
        deselect_old(false));
    }
    else{
      context.Add(m_object, m_z, select_added(then_false(m_select)),
        deselect_old(false));
    }
  }

  void Undo(CommandContext& context) override{
    context.Remove(m_object);
  }

  utf8_string Name() const override{
    return m_name + " " + m_object->GetType();
  }

private:
  Object* m_object;
  bool m_select;
  utf8_string m_name;
  int m_z;
};

Command* add_object_command(Object* obj,
  const select_added& selectAdded,
  const utf8_string& name)
{
  return new AddObjectCommand(obj, selectAdded, name, -1);
}

Command* add_object_command(Object* obj,
  const select_added& selectAdded,
  int z,
  const utf8_string& name)
{
  assert(z >= -1);
  return new AddObjectCommand(obj, selectAdded, name, z);
}

} // namespace
