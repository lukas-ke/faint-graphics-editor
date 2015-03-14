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

#include "app/active-canvas.hh"
#include "app/canvas.hh"
#include "commands/change-setting-cmd.hh"
#include "commands/command-bunch.hh"
#include "tasks/select-object-idle.hh"
#include "tools/multi-tool.hh"
#include "util/bound-setting.hh"
#include "util/object-util.hh"
#include "util/setting-id.hh"

namespace faint{

// Creates a single object update setting or a CommandBunch if
// multiple objects are affected
template<typename T>
Command* change_setting_objects(const objects_t& objects,
  const T& s,
  typename T::ValueType value)
{
  if (objects.empty()){
    return nullptr;
  }
  std::vector<Command*> commands;
  for (Object* obj : objects){
    if (obj->GetSettings().Has(s)){
      commands.push_back(change_setting_command(obj, s, value));
    }
  }
  if (commands.empty()){
    return nullptr;
  }
  return perhaps_bunch(CommandType::OBJECT,
    bunch_name(space_sep("Change", setting_name_pretty(untyped(s)),
      bracketed(get_collective_type(objects)))),
    commands);
}

// Creates a single object update setting or a CommandBunch if
// multiple objects are affected
static Command* change_settings_objects(const objects_t& objects,
  const Settings& s)
{
  if (objects.empty()){
    return nullptr;
  }
  std::vector<Command*> commands;
  for (Object* obj : objects){
    commands.push_back(change_settings_command(obj,
      New(s),
      Old(obj->GetSettings())));
  }
  return perhaps_bunch(CommandType::OBJECT,
    bunch_name("Change object settings"), commands);
}

class ObjectSelectionTool : public MultiTool {
public:
  ObjectSelectionTool(const ActiveCanvas& activeCanvas)
    : MultiTool(ToolId::OTHER,
      default_task(select_object_idle(m_settings, activeCanvas))),
      m_activeCanvas(activeCanvas)
  {
    m_settings = get_object_settings(m_activeCanvas->GetObjectSelection());
  }

  const Settings& GetSettings() const override{
    return m_settings;
  }

  bool EatsSettings() const override{
    return true;
  }

  bool Set(const BoundSetting& s) override{
    Command* cmd = nullptr;
    const auto& objects = m_activeCanvas->GetObjectSelection();

    s.Visit(
      [&](BoolSetting s, BoolSetting::ValueType v) -> bool{
        cmd = change_setting_objects(objects, s, v);
        return true;
      },
      [&](IntSetting s, IntSetting::ValueType v){
        cmd = change_setting_objects(objects, s, v);
        return true;
      },
      [&](StringSetting s, StringSetting::ValueType v){
        cmd = change_setting_objects(objects, s, v);
        return true;
      },
      [&](FloatSetting s, FloatSetting::ValueType v){
        cmd = change_setting_objects(objects, s, v);
        return true;
      },
      [&](ColorSetting s, ColorSetting::ValueType v){
        cmd = change_setting_objects(objects, s, v);
        return true;
      });

    if (cmd != nullptr){
      m_settings.Update(s);
      m_activeCanvas->RunCommand(cmd);
      return true;
    }
    return false;
  }

  bool UpdateSettings(const Settings& s) override{
    bool changed = m_settings.Update(s);
    Command* cmd =
      change_settings_objects(m_activeCanvas->GetObjectSelection(),
        m_settings);
    if (cmd != nullptr){
      m_activeCanvas->RunCommand(cmd);
    }
    return changed;
  }

private:
  ActiveCanvas m_activeCanvas;
  Settings m_settings;
};

Tool* object_selection_tool(const ActiveCanvas& active){
  return new ObjectSelectionTool(active);
}

} // namespace
