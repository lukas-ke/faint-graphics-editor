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

#ifndef FAINT_MERGE_CONDITION_HH
#define FAINT_MERGE_CONDITION_HH
#include <memory>

namespace faint{ class Command; }

namespace faint{

class MergeCondition{
  // Specifies conditions for merging all commands in two
  // CommandBunches, and for appending a single command to an existing
  // CommandBunch.
public:
  virtual ~MergeCondition() = default;

  // True if the command should be appended to the list of commands in
  // the command bunch with this merge condition.
  virtual bool ShouldAppend(const Command&) const = 0;
  virtual void NotifyAppended() = 0;

  // True if the command bunch should assume the name of the
  // merged command bunch or appended command.
  virtual bool AssumeName() const = 0;

  // True if all commands should be merged.
  virtual bool Satisfied(const MergeCondition&) = 0;
  bool Unsatisfied(const MergeCondition& other);
};

using MergeConditionPtr = std::unique_ptr<MergeCondition>;

} // namespace

#endif
