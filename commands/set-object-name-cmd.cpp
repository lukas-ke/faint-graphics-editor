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

#include "commands/obj-function-cmd.hh"
#include "commands/set-object-name-cmd.hh"
#include "objects/object.hh"
#include "text/formatting.hh"

namespace faint{

Optional<utf8_string> empty_to_unset(const Optional<utf8_string>& name){
  return name.Visit(
    [](const utf8_string& s) -> Optional<utf8_string>{
      return {s, !s.empty()};
    },
    []() -> Optional<utf8_string>{
      return {};
    });
}

void set_name(Object* obj, const Optional<utf8_string>& name){
  obj->SetName(name);
}

Command* set_object_name_command(Object* obj,
  const Optional<utf8_string>& name)
{
  using SetNameCmd = ObjFunctionCommand<Object,
                                        Optional<utf8_string>,
                                        set_name>;

  // Use "Rename .." for the command name if changing.
  // "Remove .." if setting to nothing.
  // "Name .." if changing from nothing to new name
  const utf8_string commandName = name.Visit(
     [&](const utf8_string& name){
       return obj->GetName().Visit(
         [&](const utf8_string& oldName){
           return space_sep("Rename", oldName, "to", name);
         },
         [&](){
           return space_sep("Name", obj->GetType(), name);
         });
     },
     [&](){
       return obj->GetName().Visit(
         [&](const utf8_string& oldName){
           return space_sep("Remove Name", oldName);
         },
         [&](){
           // Should preferably not use a command for clearing an
           // already cleared object name.
           return space_sep("Clear", obj->GetType(), "Name");
         });});

  return new SetNameCmd(obj, commandName, New(name),
    Old(obj->GetName()));
}

} // namespace
