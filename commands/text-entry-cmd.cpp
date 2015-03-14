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

#include "commands/command.hh"
#include "objects/objtext.hh"
#include "commands/text-entry-cmd.hh"
#include "text/utf8-string.hh"

namespace faint{

class TextEntryCommand : public Command{
public:
  TextEntryCommand(ObjText* textObj, const NewText& newText,
    const OldText& oldText)
    : Command(CommandType::OBJECT),
      m_textObj(textObj),
      m_old(oldText.Get()),
      m_new(newText.Get())
  {}

  void Do(CommandContext&) override{
    m_textObj->GetTextBuffer().set(m_new);
  }

  utf8_string Name() const override{
    return "Edit Text";
  }

  void Undo(CommandContext&) override{
    m_textObj->GetTextBuffer().set(m_old);
  }

  TextEntryCommand& operator=(const TextEntryCommand&) = delete;
private:
  ObjText* m_textObj;
  const utf8_string m_old;
  const utf8_string m_new;
};

Command* text_entry_command(ObjText* obj, const NewText& newText,
  const OldText& oldText)
{
  return new TextEntryCommand(obj, newText, oldText);
}

} // namespace
