// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
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

#include "geo/line.hh"
#include "tasks/calibrate-tasks.hh"
#include "tools/multi-tool.hh"

namespace faint{

class CalibrateTool : public MultiTool{
public:
  CalibrateTool()
    : MultiTool(ToolId::CALIBRATE, default_task(calibrate_idle()))
  {}

  bool EatsSettings() const override{
    return false;
  }

  const Settings& GetSettings() const override{
    return m_settings;
  }

  bool UpdateSettings(const Settings&) override{
    return false;
  }

  bool Set(const BoundSetting&) override{
    return false;
  }

private:
  PendingCommand m_command;
  Settings m_settings;
};

Tool* calibrate_tool(){
  return new CalibrateTool();
}

} // namespace
