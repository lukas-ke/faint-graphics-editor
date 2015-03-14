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

#ifndef FAINT_BITMAP_CMD_HH
#define FAINT_BITMAP_CMD_HH

namespace faint{

class Bitmap;
class IntRect;
class utf8_string;

class BitmapCommand{
  // A limited command-variant, deliberately unrelated to Command.
  //
  // These commands are only able to change the pixel content of a
  // bitmap.
public:
  virtual ~BitmapCommand() = default;
  virtual utf8_string Name() const = 0;
  virtual void Do(Bitmap&) = 0;
};

class Command;

// Wrappers for assigning targets for a BitmapCommand.  The returned
// command manages deletion of the BitmapCommand.
Command* target_full_image(BitmapCommand*);
Command* target_floating_selection(BitmapCommand*);
Command* target_rectangle(BitmapCommand*, const IntRect&);

} // namespace

#endif
