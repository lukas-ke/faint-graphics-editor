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
#include "commands/remove-point-cmd.hh"
#include "geo/point.hh"
#include "objects/object.hh"
#include "text/formatting.hh"

namespace faint{

class RemovePointCommand : public Command {
public:
  RemovePointCommand(Object* object, int pointIndex)
    : Command(CommandType::OBJECT),
      m_object(object),
      m_pointIndex(pointIndex),
      m_point(object->GetPoint(pointIndex))
  {}

  void Do(CommandContext&) override{
    m_object->RemovePoint(m_pointIndex);
  }

  utf8_string Name() const override{
    return space_sep("Delete", m_object->GetType(), "Point");
  }

  void Undo(CommandContext&) override{
    m_object->InsertPoint(m_point, m_pointIndex);
  }

  RemovePointCommand& operator=(const RemovePointCommand&) = delete;

private:
  Object* m_object;
  int m_pointIndex;
  Point m_point;
};

Command* remove_point_command(Object* object, int pointIndex){
  return new RemovePointCommand(object, pointIndex);
}

} // namespace
