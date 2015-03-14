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
#include "commands/draw-object-cmd.hh"
#include "objects/object.hh"
#include "text/formatting.hh"
#include "util/image.hh"

namespace faint{

class DrawObjectCommand : public Command{
public:
  DrawObjectCommand(const its_yours_t<Object>& obj)
    : Command(CommandType::RASTER),
      m_delete(true),
      m_object(obj.Get())
  {}

  DrawObjectCommand(const just_a_loan_t<Object>& obj)
    : Command(CommandType::RASTER),
      m_delete(false),
      m_object(obj.Get())
  {}

  ~DrawObjectCommand(){
    if (m_delete){
      delete m_object;
    }
  }

  void Do(CommandContext& context) override{
    m_object->Draw(context.GetDC(),
      context.GetFrame().GetExpressionContext());
  }

  utf8_string Name() const override{
    return space_sep("Draw ", m_object->GetType());
  }
private:
  bool m_delete;
  Object* m_object;
};

Command* draw_object_command(const its_yours_t<Object>& object){
  return new DrawObjectCommand(object);
}

Command* draw_object_command(const just_a_loan_t<Object>& object){
  return new DrawObjectCommand(object);
}

} // namespace
