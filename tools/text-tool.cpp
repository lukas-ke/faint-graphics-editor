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

#include "tasks/text-idle.hh"
#include "tools/multi-tool.hh"
#include "util/default-settings.hh"

namespace faint{

static Settings text_tool_settings(const Settings& allSettings){
  Settings s(default_text_settings());
  s.Update(allSettings);
  return s;
}

class TextTool : public MultiTool{
public:
  TextTool(const Settings& allSettings)
    : MultiTool(ToolId::TEXT,
      default_task(text_idle_task(m_settings))),
      m_settings(text_tool_settings(allSettings))
  {}

  bool EatsSettings() const override{
    return false;
  }

  const Settings& GetSettings() const override{
    return m_settings;
  }

  bool Set(const BoundSetting& s) override{
    if (m_settings.Update(s)){
      UpdateTaskSettings();
      return true;
    }
    return false;
  }

  bool UpdateSettings(const Settings& s) override{
    if (m_settings.Update(s)){
      UpdateTaskSettings();
      return true;
    }
    return false;
  }

private:
  Settings m_settings;
};

Tool* text_tool(const Settings& allSettings){
  return new TextTool(allSettings);
}

} // namespace
