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

#ifndef FAINT_MOVE_POINT_CMD_HH
#define FAINT_MOVE_POINT_CMD_HH
#include "commands/command.hh"
#include "geo/point.hh"
#include "util/distinct.hh"

namespace faint{

using NewPoint = Order<Point>::New;
using OldPoint = Order<Point>::Old;

class MovePointCommand : public Command {
public:
  MovePointCommand(Object*, int pointIndex, const NewPoint&, const OldPoint&);
  utf8_string Name() const override;
  void Do(CommandContext&) override;
  void Undo(CommandContext&) override;
private:
  MovePointCommand& operator=(const MovePointCommand&);
  Object* m_object;
  int m_pointIndex;
  const Point m_new;
  const Point m_old;
};

} // namespace

#endif
