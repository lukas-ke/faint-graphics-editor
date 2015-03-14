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

#include "editors/text-entry-util.hh"
#include "text/char-constants.hh"
#include "text/text-buffer.hh"
#include "util-wx/key-codes.hh"

namespace faint{

static bool select(const Mod& mod){
  return mod.Shift();
}

static bool handle_special_key(const KeyPress& key, TextBuffer& text){
  if (key.Is(key::back)){
    text.del_back();
    return true;
  }
  else if (key.Is(key::del)){
    text.del();
    return true;
  }
  else if (key.Is(key::enter)){
    text.insert(eol);
    return true;
  }
  else if (key.Is(key::left)){
    text.devance(select(key.Modifiers()));
    return true;
  }
  else if (key.Is(key::right)){
    text.advance(select(key.Modifiers()));
    return true;
  }
  else if (key.Is(key::up)){
    text.move_up(select(key.Modifiers()));
    return true;
  }
  else if (key.Is(key::down)){
    text.move_down(select(key.Modifiers()));
    return true;
  }
  else if (key.Is(key::home)){
    size_t newPos = text.prev(eol);
    text.caret(newPos == 0 ? newPos : newPos + 1, select(key.Modifiers()));
    return true;
  }
  else if (key.Is(key::end)){
    text.caret(text.next(eol), select(key.Modifiers()));
    return true;
  }
  else if (key.Is(Ctrl, key::T)){
    transpose_chars(text);
    return true;
  }
  return false;
}

static bool printable_char(const utf8_char& ch){
  if (ch == replacement_character || ch == utf8_null){
    return false;
  }
  return true;
}

bool handle_key_press(const KeyPress& key, TextBuffer& text){
  if (handle_special_key(key, text)){
    return true;
  }
  else if (key.Ctrl() && !key.Alt()){
    // Ctrl is for menu shortcuts. On wxGtk menu is
    // evaluated after panels, so if this function consumes
    // menu shortcuts, they won't happen.
    //
    // However: Exclude alt+ctrl, as this seems to mean
    // alt+gr occasionally See \ref(alt-xubuntu).
    return false;
  }
  else if (printable_char(key.Char())){
    text.insert(key.Char());
    return true;
  }
  return false;
}

} // namespace
