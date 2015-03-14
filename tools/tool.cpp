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

#include "tools/tool.hh"
#include "util/optional.hh"

namespace faint{

bool HistoryContext::PreventsGlobalRedo() const{
  return !AllowsGlobalRedo();
}

Tool::Tool(ToolId id)
  : m_id(id)
{}

ToolId Tool::GetId() const{
  return m_id;
}

Optional<faint::HistoryContext&> Tool::HistoryContext() {
  return const_cast<const Tool*>(this)->HistoryContext().Visit(
    [](const faint::HistoryContext& c){
      return
        Optional<faint::HistoryContext&>(const_cast<faint::HistoryContext&>(c));
    },
    [](){
      return Optional<faint::HistoryContext&>();
    });
}

Optional<faint::SelectionContext&> Tool::SelectionContext() {
  return const_cast<const Tool*>(this)->SelectionContext().Visit(
    [](const faint::SelectionContext& c){
      return Optional<faint::SelectionContext&>(
        const_cast<faint::SelectionContext&>(c));
    },
    [](){
      return Optional<faint::SelectionContext&>();
    });
}


Optional<faint::TextContext&> Tool::TextContext() {
  return const_cast<const Tool*>(this)->TextContext().Visit(
    [](const faint::TextContext& c){
      return Optional<faint::TextContext&>(const_cast<faint::TextContext&>(c));
    },
    [](){
      return Optional<faint::TextContext&>();
    });
}

} // namespace
