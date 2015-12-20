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

#include "util/merge-conditions.hh"

namespace faint{

class AppendOnceIf : public MergeCondition{
public:
  AppendOnceIf(const CommandPredicate& condition, faint::AssumeName assumeName)
    : m_appended(false),
      m_assumeName(assumeName),
      m_condition(condition)
  {}

  bool Satisfied(const MergeCondition&) override{
    // AppendOnceIf is only used for appending, not merging
    // CommandBunches.
    return false;
  }

  bool ShouldAppend(const Command& cmd) const override{
    return !m_appended && m_condition(cmd);
  }

  void NotifyAppended() override{
    m_appended = true;
  }

  bool AssumeName() const override{
    return m_assumeName == faint::AssumeName::Yes;
  }

private:
  bool m_appended;
  faint::AssumeName m_assumeName;
  std::function<bool(const Command&)> m_condition;
};

MergeConditionPtr append_once_if(const std::function<bool(const Command&)>& f,
  AssumeName assumeName)
{
  return std::make_unique<AppendOnceIf>(f, assumeName);
}

} // namespace
