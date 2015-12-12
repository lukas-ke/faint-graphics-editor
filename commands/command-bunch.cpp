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

namespace faint{

class CommandBunch : public Command{
public:
  CommandBunch(CommandType type,
    std::vector<CommandPtr> commands,
    const bunch_name& name,
    MergeCondition* mergeCondition=nullptr)
    : Command(type),
      m_commands(std::move(commands)),
      m_name(name.Get()),
      m_mergeCondition(mergeCondition)
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

  bool Merge(CommandPtr& cmd, bool sameFrame) override{
    if (!sameFrame){
      return false;
    }

    if (m_mergeCondition == nullptr){
      return false;
    }

    if (m_mergeCondition->Append(cmd)){
      m_commands.emplace_back(std::move(cmd));
      if (m_mergeCondition->AssumeName()){
        m_name = cmd->Name();
      }
      return true;
    }

    auto* candidate = dynamic_cast<CommandBunch*>(cmd.get());
    if (candidate == nullptr){
      return false;
    }
    if (candidate->m_mergeCondition == nullptr){
      return false;
    }
    if (candidate->m_commands.size() != m_commands.size()){
      return false;
    }
    if (m_mergeCondition->Unsatisfied(candidate->m_mergeCondition.get())){
      return false;
    }

    for (size_t i = 0; i != m_commands.size(); i++){
      bool merged = m_commands[i]->Merge(candidate->m_commands[i], sameFrame);
      assert(merged);
    }
    if (m_mergeCondition->AssumeName()){
      m_name = candidate->Name();
    }
    return true;
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
  std::vector<CommandPtr> m_commands;
  utf8_string m_name;
  std::unique_ptr<MergeCondition> m_mergeCondition;
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
  std::vector<CommandPtr> commands)
{
  assert(!commands.empty());
  if (commands.size() == 1){
    return first_element(commands);
  }
  else{
    return std::make_unique<CommandBunch>(type, std::move(commands), name);
  }
}

std::vector<CommandPtr> move_to_vec(std::deque<CommandPtr>& cmds){
  std::vector<CommandPtr> v;
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
  std::vector<CommandPtr> commands,
  MergeCondition* mergeCondition)
{
  return std::make_unique<CommandBunch>(type, std::move(commands),
    name,
    mergeCondition);
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
  MergeCondition* mergeCondition)
{
  std::vector<CommandPtr> v;
  v.emplace_back(std::move(cmd));
  return std::make_unique<CommandBunch>(type, std::move(v), name,
    mergeCondition);
}

CommandPtr command_bunch(CommandType type,
  const bunch_name& name,
  CommandPtr cmd1,
  CommandPtr cmd2,
  MergeCondition* mergeCondition)
{
  std::vector<CommandPtr> v;
  v.emplace_back(std::move(cmd1));
  v.emplace_back(std::move(cmd2));
  return std::make_unique<CommandBunch>(type, std::move(v), name, mergeCondition);
}

CommandPtr command_bunch(CommandType type,
  const bunch_name& name,
  CommandPtr cmd,
  std::unique_ptr<MergeCondition> mergeCondition)
{
  return command_bunch(type,
    name,
    std::move(cmd),
    mergeCondition.release());
}

bool MergeCondition::Unsatisfied(MergeCondition* other){
  return !Satisfied(other);
}

} // namespace
