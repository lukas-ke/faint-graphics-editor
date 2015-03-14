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

#ifndef FAINT_TEXT_ENTRY_UTIL_HH
#define FAINT_TEXT_ENTRY_UTIL_HH

namespace faint{

class KeyPress;
class TextBuffer;

// Modifies the TextBuffer depending on the keypress. Characters are
// appended at the caret position. Navigation keys adjust the caret
// position or selection. Some special keys, like delete and backspace
// are also handled.
//
// Returns true if the keypress modified the TextBuffer in any way.
bool handle_key_press(const KeyPress&, TextBuffer&);

} // namespace

#endif
