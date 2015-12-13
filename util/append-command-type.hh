// -*- coding: us-ascii-unix -*-
// Copyright 2015 Lukas Kemmer
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

#ifndef FAINT_APPEND_COMMAND_TYPE
#define FAINT_APPEND_COMMAND_TYPE
#include "commands/command-bunch.hh" // MergeCondition

namespace faint{

template<typename T>
class AppendCommandType : public MergeCondition{
  // Condition for appending a command of a certain type to a
  // command-bunch.
public:
  AppendCommandType()
    : m_appended(false)
  {}

  bool Satisfied(MergeCondition*) override{
    // AppendCommandType is only used for appending, not merging
    // CommandBunches.
    return false;
  }

  bool Append(CommandPtr& cmd) override{
    if (m_appended){
      // Only append a single command
      return false;
    }
    // Only attempt appending once
    m_appended = true;

    return dynamic_cast<T*>(cmd.get()) != nullptr;
  }

  bool AssumeName() const override{
    return false;
  }

private:
  bool m_appended;
};

} // namespace

#endif