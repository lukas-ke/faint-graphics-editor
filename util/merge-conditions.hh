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

#ifndef FAINT_MERGE_CONDITIONS_HH
#define FAINT_MERGE_CONDITIONS_HH
#include <functional>
#include "commands/merge-condition.hh"

namespace faint{

enum class AssumeName{Yes, No};

using CommandPredicate = std::function<bool(const Command&)>;

MergeConditionPtr append_once_if(const CommandPredicate&, AssumeName);

} // namespace

#endif
