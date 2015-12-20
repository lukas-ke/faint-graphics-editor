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

#include <cassert>
#include <memory>
#include "commands/command.hh"
#include "commands/command-bunch.hh"
#include "text/utf8-string.hh"
#include "util/iter.hh"
#include "util/type-util.hh"

namespace faint{

class CommandBunch : public Command{
public:
  CommandBunch(CommandType type,
    commands_t commands,
    const bunch_name& name,
    MergeConditionPtr mergeCondition=nullptr)
    : Command(type),
      m_commands(std::move(commands)),
      m_name(name.Get()),
      m_mergeCondition(std::move(mergeCondition))
  {
    assert(!m_commands.empty());
  }

  void Do(CommandContext& context) override{
    for (auto& cmd : m_commands){
      cmd->Do(context);
    }
  }

  void DoRaster(CommandContext& context) override{
    for (auto& cmd : m_commands){
      cmd->DoRaster(context);
    }
  }

  bool ShouldMerge(const Command& cmd, bool sameFrame) const override{
    if (!sameFrame){
      return false;
    }

    if (m_mergeCondition == nullptr){
      return false;
    }

    if (m_mergeCondition->ShouldAppend(cmd)){
      return true;
    }

    return if_type<const CommandBunch>(cmd,
      [&](const CommandBunch& cmd){
        return
          cmd.m_mergeCondition != nullptr &&
          cmd.m_commands.size() == m_commands.size() &&
          m_mergeCondition->Satisfied(*cmd.m_mergeCondition);
      },
      [](){
        return false;
      });
  }

  void Merge(CommandPtr cmd) override{
    assert(m_mergeCondition != nullptr);

    if (m_mergeCondition->ShouldAppend(*cmd)){
      DoAppend(std::move(cmd));
    }
    else{
      DoMerge(unique_ptr_cast<CommandBunch>(std::move(cmd)));
    }
  }

  utf8_string Name() const override{
    return m_name.empty() ? m_commands.back()->Name() : m_name;
  }

  void Undo(CommandContext& context) override{
    for (auto& cmd : reversed(m_commands)){
      cmd->Undo(context);
    }
  }

private:
  void DoAppend(CommandPtr cmd){
    assert(m_mergeCondition != nullptr);
    assert(m_mergeCondition->ShouldAppend(*cmd));
    if (m_mergeCondition->AssumeName()){
       m_name = cmd->Name();
    }
    m_commands.push_back(std::move(cmd));
    m_mergeCondition->NotifyAppended();
  }

  void DoMerge(std::unique_ptr<CommandBunch> cmd){
    if (m_mergeCondition->AssumeName()){
      m_name = cmd->Name();
    }

    for (size_t i = 0; i != m_commands.size(); i++){
      m_commands[i]->Merge(std::move(cmd->m_commands[i]));
    }
  }

  commands_t m_commands;
  utf8_string m_name;
  MergeConditionPtr m_mergeCondition;
};

template<typename T>
CommandPtr first_element(T& commands){
  assert(!commands.empty());
  auto it = begin(commands);
  CommandPtr p = std::move(*it);
  commands.erase(it);
  return p;
}

CommandPtr perhaps_bunch(CommandType type,
  const bunch_name& name,
  commands_t commands)
{
  assert(!commands.empty());
  if (commands.size() == 1){
    return first_element(commands);
  }
  else{
    return std::make_unique<CommandBunch>(type, std::move(commands), name);
  }
}

commands_t move_to_vec(std::deque<CommandPtr>& cmds){
  commands_t v;
  v.reserve(cmds.size());
  for (auto it = begin(cmds); it != end(cmds); it = cmds.erase(it)){
    v.emplace_back(std::move(*it));
  }
  return v;
}

CommandPtr perhaps_bunch(CommandType type,
  const bunch_name& name,
  std::deque<CommandPtr> commands)
{
  assert(!commands.empty());
  if (commands.size() == 1){
    return first_element(commands);
  }
  else{
    return std::make_unique<CommandBunch>(type,
      move_to_vec(commands), name);
  }
}

CommandPtr command_bunch(CommandType type,
  const bunch_name& name,
  commands_t commands,
  MergeConditionPtr mergeCondition)
{
  return std::make_unique<CommandBunch>(type, std::move(commands),
    name,
    std::move(mergeCondition));
}

CommandPtr command_bunch(CommandType type,
  const bunch_name& name,
  std::deque<CommandPtr> commands)
{
  return std::make_unique<CommandBunch>(type,
    move_to_vec(commands),
    name);
}

CommandPtr command_bunch(CommandType type,
  const bunch_name& name,
  CommandPtr cmd,
  MergeConditionPtr mergeCondition)
{
  commands_t v;
  v.emplace_back(std::move(cmd));
  return std::make_unique<CommandBunch>(type,
    std::move(v),
    name,
    std::move(mergeCondition));
}

CommandPtr command_bunch(CommandType type,
  const bunch_name& name,
  CommandPtr cmd1,
  CommandPtr cmd2,
  MergeConditionPtr mergeCondition)
{
  commands_t v;
  v.emplace_back(std::move(cmd1));
  v.emplace_back(std::move(cmd2));
  return std::make_unique<CommandBunch>(type,
    std::move(v),
    name,
    std::move(mergeCondition));
}

} // namespace
