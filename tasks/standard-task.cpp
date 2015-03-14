// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
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

#include "tasks/standard-task.hh"
#include "util/optional.hh"

namespace faint{

void StandardTask::Activate(){
}

TaskResult StandardTask::Char(const KeyInfo&){
  return TaskResult::NONE;
}

TaskResult StandardTask::DoubleClick(const PosInfo& info){
  // Default to forwarding the double click as another left down
  return MouseDown(info);
}

bool StandardTask::EatsSettings() const{
  return false;
}

Optional<const faint::HistoryContext&> StandardTask::HistoryContext() const{
  return {};
}

bool StandardTask::RefreshOnMouseOut() const{
  return false;
}

Optional<const faint::SelectionContext&> StandardTask::SelectionContext() const{
  return {};
}

void StandardTask::SelectionChange(){
}

void StandardTask::SetLayer(Layer){
}

Optional<const faint::TextContext&> StandardTask::TextContext() const{
  return {};
}

void StandardTask::UpdateSettings(){
}

} // namespace
