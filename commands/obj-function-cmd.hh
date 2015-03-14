// -*- coding: us-ascii-unix -*-
// Copyright 2013 Lukas Kemmer
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

#ifndef FAINT_OBJ_FUNCTION_CMD_HH
#define FAINT_OBJ_FUNCTION_CMD_HH
#include "commands/command.hh"
#include "text/utf8-string.hh"

namespace faint{

template<typename OBJ_T, typename T1, void Func(OBJ_T*, const T1&)>
class ObjFunctionCommand : public Command{
  // Command which calls a function with the object and the "new"
  // argument on Do and the "old" argument on Undo.
public:
  ObjFunctionCommand(OBJ_T* object, const utf8_string& name,
    const typename Order<T1>::New& newArg,
    const typename Order<T1>::Old& oldArg)
    : Command(CommandType::OBJECT),
      m_name(name),
      m_object(object),
      m_newArg(newArg.Get()),
      m_oldArg(oldArg.Get())
  {}

  void Do(CommandContext&) override{
    Func(m_object, m_newArg);
  }

  utf8_string Name() const override{
    return m_name;
  }

  void Undo(CommandContext&) override{
    Func(m_object, m_oldArg);
  }
private:
  utf8_string m_name;
  OBJ_T* m_object;
  T1 m_newArg;
  T1 m_oldArg;
};

} // namespace

#endif
