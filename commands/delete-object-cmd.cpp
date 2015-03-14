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
#include "commands/delete-object-cmd.hh"
#include "objects/object.hh"
#include "text/formatting.hh"

namespace faint{

class DeleteObjectCommand : public Command {
public:
  DeleteObjectCommand(Object* object, int objectZ, const utf8_string& name)
    : Command(CommandType::OBJECT),
      m_name(name),
      m_object(object),
      m_objectZ(objectZ)
  {}

  void Do(CommandContext& context) override{
    context.Remove(m_object);
  }

  utf8_string Name() const override{
    return space_sep(m_name, m_object->GetType());
  }

  void Undo(CommandContext& context) override{
    context.Add(m_object, m_objectZ, select_added(false), deselect_old(false));
  }
private:
  utf8_string m_name;
  Object* m_object;
  int m_objectZ;
};

Command* delete_object_command(Object* object, int z, const utf8_string& name){
  return new DeleteObjectCommand(object, z, name);
}

} // namespace
