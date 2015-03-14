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
#include "commands/order-object-cmd.hh"
#include "objects/object.hh"
#include "text/formatting.hh"

namespace faint{

static bool is_forward_move(const NewZ& newZ, const OldZ& oldZ){
  return newZ.Get() > oldZ.Get();
}

utf8_string forward_or_back_str(const NewZ& newZ, const OldZ& oldZ){
  return is_forward_move(newZ, oldZ) ? "Forward" : "Back";
}

class OrderObjectCommand : public Command {
public:
  OrderObjectCommand(Object* object, const NewZ& newZ, const OldZ& oldZ)
    : Command(CommandType::OBJECT),
      m_object(object),
      m_newZ(newZ),
      m_oldZ(oldZ)
  {}

  void Do(CommandContext& context) override{
    context.SetObjectZ(m_object, m_newZ.Get());
  }

  utf8_string Name() const override{
    return space_sep(m_object->GetType(), forward_or_back_str(m_newZ, m_oldZ));
  }

  void Undo(CommandContext& context) override{
    context.SetObjectZ(m_object, m_oldZ.Get());
  }
private:
  Object* m_object;
  NewZ m_newZ;
  OldZ m_oldZ;
};

Command* order_object_command(Object* object, const NewZ& newZ, const OldZ& oldZ){
  return new OrderObjectCommand(object,newZ, oldZ);
}

} // namespace
