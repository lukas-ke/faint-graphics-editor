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

#include <memory>
#include "commands/add-object-cmd.hh"
#include "commands/command.hh"
#include "objects/object.hh"
#include "util/convenience.hh"

namespace faint{

class AddObjectCommand : public Command {
public:
  AddObjectCommand(Object* object,
    const select_added& select,
    const Optional<int>& z,
    const utf8_string& name)
    : Command(CommandType::OBJECT),
      m_name(name),
      m_object(object),
      m_select(select.Get()),
      m_z(z)
  {}

  void Do(CommandContext& context) override{
    m_z.Visit(
      [&](int z){
        // Insert at the given z-index
        context.Add(m_object.get(), z, select_added(then_false(m_select)),
          deselect_old(false));
      },
      [&](){
        context.Add(m_object.get(), select_added(then_false(m_select)),
          deselect_old(false));
      });
  }

  void Undo(CommandContext& context) override{
    context.Remove(m_object.get());
  }

  utf8_string Name() const override{
    return m_name + " " + m_object->GetType();
  }

private:
  utf8_string m_name;
  std::unique_ptr<Object> m_object;
  bool m_select;
  Optional<int> m_z;
};

Command* add_object_command(Object* obj,
  const select_added& selectAdded,
  const utf8_string& name)
{
  return new AddObjectCommand(obj, selectAdded, no_option(), name);
}

Command* insert_object_command(Object* obj,
  const select_added& selectAdded,
  int z,
  const utf8_string& name)
{
  assert(z >= 0);
  return new AddObjectCommand(obj, selectAdded, option(z), name);
}

} // namespace
