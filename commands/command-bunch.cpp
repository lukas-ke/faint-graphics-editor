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
#include "commands/command-bunch.hh"
#include "commands/command.hh"
#include "text/utf8-string.hh"
#include "util/iter.hh"

namespace faint{

class CommandBunch : public Command{
public:
  CommandBunch(CommandType type,
    const commands_t& commands,
    const bunch_name& name,
    MergeCondition* mergeCondition=nullptr)
    : Command(type),
      m_commands(commands),
      m_name(name.Get()),
      m_mergeCondition(mergeCondition)
  {
    assert(!commands.empty());
  }

  ~CommandBunch(){
    for (Command* cmd : m_commands){
      delete cmd;
    }
  }

  void Do(CommandContext& context) override{
    for (Command* cmd : m_commands){
      cmd->Do(context);
    }
  }

  void DoRaster(CommandContext& context) override{
    for (Command* cmd : m_commands){
      cmd->DoRaster(context);
    }
  }

  bool Merge(Command* cmd, bool sameFrame) override{
    if (!sameFrame){
      return false;
    }

    if (m_mergeCondition == nullptr){
      return false;
    }

    if (m_mergeCondition->Append(cmd)){
      m_commands.push_back(cmd);
      if (m_mergeCondition->AssumeName()){
        m_name = cmd->Name();
      }
      return true;
    }

    CommandBunch* candidate = dynamic_cast<CommandBunch*>(cmd);
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
    for (Command* cmd : reversed(m_commands)){
      cmd->Undo(context);
    }
  }
private:
  std::vector<Command*> m_commands;
  utf8_string m_name;
  std::unique_ptr<MergeCondition> m_mergeCondition;
};

Command* perhaps_bunch(CommandType type,
  const bunch_name& name,
  const commands_t& commands)
{
  assert(!commands.empty());
  return (commands.size() == 1) ? commands.front() :
    new CommandBunch(type, commands, name);
}

Command* perhaps_bunch(CommandType type,
  const bunch_name& name,
  const std::deque<Command*>& commands)
{
  assert(!commands.empty());
  return (commands.size() == 1) ? commands.front() :
    new CommandBunch(type,
      std::vector<Command*>(begin(commands), end(commands)), name);
}

Command* command_bunch(CommandType type,
  const bunch_name& name,
  const commands_t& commands,
  MergeCondition* mergeCondition)
{
  return new CommandBunch(type, commands, name, mergeCondition);
}

Command* command_bunch(CommandType type,
  const bunch_name& name,
  const std::deque<Command*>& commands)
{
  return new CommandBunch(type,
    std::vector<Command*>(begin(commands), end(commands)),
    name);
}

Command* command_bunch(CommandType type,
  const bunch_name& name,
  Command* cmd,
  MergeCondition* mergeCondition)
{
  return new CommandBunch(type, {cmd}, name, mergeCondition);
}

Command* command_bunch(CommandType type,
  const bunch_name& name,
  Command* cmd1,
  Command* cmd2,
  MergeCondition* mergeCondition)
{
  return new CommandBunch(type, {cmd1, cmd2}, name, mergeCondition);
}

bool MergeCondition::Unsatisfied(MergeCondition* other){
  return !Satisfied(other);
}

} // namespace
