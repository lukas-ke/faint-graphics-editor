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

#include "commands/add-point-cmd.hh"
#include "commands/command.hh"
#include "geo/point.hh"
#include "objects/object.hh"
#include "text/utf8-string.hh"

namespace faint{

class AddPointCommand : public Command{
public:
  AddPointCommand(Object* object, int pointIndex, const Point& point)
    : Command(CommandType::OBJECT),
      m_object(object),
      m_pointIndex(pointIndex),
      m_point(point)
  {}

  void Do(CommandContext&) override{
    m_object->InsertPoint(m_point, m_pointIndex);
  }

  utf8_string Name() const override{
    return "Add Point to " + m_object->GetType();
  }

  void Undo(CommandContext&) override{
    m_object->RemovePoint(m_pointIndex);
  }

  AddPointCommand& operator=(const AddPointCommand&) = delete;

private:
  Object* m_object;
  int m_pointIndex;
  Point m_point;
};

Command* add_point_command(Object* object, int pointIndex, const Point& point){
  return new AddPointCommand(object, pointIndex, point);
}

} // namespace
