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

#ifndef FAINT_OLD_COMMAND_HH
#define FAINT_OLD_COMMAND_HH
#include "text/utf8-string.hh"
#include "util/optional.hh"

namespace faint{

class Command;
class Image;

enum class UndoType{
  // An OldCommand can be an actual command or the start or end of a
  // group of commands.
  OPEN_GROUP,
  CLOSE_GROUP,
  NORMAL_COMMAND
};

class OldCommand{
  // A previously ran command, with the image it targetted.
public:
  // Creates an OldCommand which signifies the start of a grouped set
  // of commands.
  static OldCommand OpenGroup();

  // Creates an OldCommand which signifies the end of a grouped set of
  // commands.
  static OldCommand CloseGroup();

  // Same as CloseGroup(), but allows giving a name to the group.
  static OldCommand CloseGroup(const utf8_string& name);

  // Creates an OldCommand which maps an executed Command to an Image,
  // so that undo/redo can target the same image.
  OldCommand(Command*, Image*);

  // Attempts to merge the passed in OldCommand with this one,
  // returns true if successful.
  bool Merge(OldCommand&);

  Command* command;
  Image* targetFrame;
  UndoType type;
  Optional<utf8_string> name;
private:
  explicit OldCommand(UndoType);
};

} // namespace

#endif
