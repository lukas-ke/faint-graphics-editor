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
#include "util/make-vector.hh"
#include "util/object-util.hh"
#include "util/setting-id.hh"

namespace faint{

// Creates a single object update setting or a CommandBunch if
// multiple objects are affected
template<typename T>
CommandPtr change_setting_objects(const objects_t& objects,
  const T& s,
  typename T::ValueType value)
{
  if (objects.empty()){
    return nullptr;
  }

  commands_t commands;
  for (Object* obj : objects){
    if (obj->GetSettings().Has(s)){
      commands.emplace_back(change_setting_command(obj, s, value));
    }
  }

  if (commands.empty()){
    return nullptr;
  }
  return perhaps_bunch(CommandType::OBJECT,
    bunch_name(space_sep("Change", setting_name_pretty(untyped(s)),
      bracketed(get_collective_type(objects)))),
    std::move(commands));
}

// Creates a single object update setting or a CommandBunch if
// multiple objects are affected
static CommandPtr change_settings_objects(const objects_t& objects,
  const Settings& s)
{
  if (objects.empty()){
    return nullptr;
  }

  auto change = [&](Object* obj){
    return change_settings_command(obj, New(s), Old(obj->GetSettings()));
  };

  return perhaps_bunch(CommandType::OBJECT,
    bunch_name("Change object settings"),
    make_vector(objects, change));
}

class ObjectSelectionTool : public MultiTool {
public:
  explicit ObjectSelectionTool(const ActiveCanvas& activeCanvas,
    ToolActions& actions)
    : MultiTool(ToolId::OTHER,
      default_task(select_object_idle(m_settings, activeCanvas, actions))),
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
    const auto& objects = m_activeCanvas->GetObjectSelection();

    auto makeCmd = [&](const auto& s, const auto& v){
      return change_setting_objects(objects, s, v);
    };

    CommandPtr cmd = s.Visit(makeCmd);
    if (cmd != nullptr){
      m_settings.Update(s);
      m_activeCanvas->RunCommand(std::move(cmd));
      return true;
    }
    return false;
  }

  bool UpdateSettings(const Settings& s) override{
    const bool changed = m_settings.Update(s);

    auto cmd = change_settings_objects(
      m_activeCanvas->GetObjectSelection(),
      m_settings);

    if (cmd != nullptr){
      m_activeCanvas->RunCommand(std::move(cmd));
    }
    return changed;
  }

private:
  ActiveCanvas m_activeCanvas;
  Settings m_settings;
};

Tool* object_selection_tool(const ActiveCanvas& active, ToolActions& actions){
  return new ObjectSelectionTool(active, actions);
}

} // namespace
