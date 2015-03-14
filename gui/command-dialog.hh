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

#ifndef FAINT_COMMAND_DIALOG_HH
#define FAINT_COMMAND_DIALOG_HH
#include <functional>
#include "commands/bitmap-cmd.hh"
#include "util/template-fwd.hh"

class wxWindow;

namespace faint{

class Canvas;
class Command;

class DialogFeedback{
  // Context for letting dialogs show feedback on a Bitmap
public:
  virtual ~DialogFeedback() = default;

  // Returns a bitmap for drawing feedback.
  virtual const Bitmap& GetBitmap() = 0;
  virtual void SetBitmap(const Bitmap&) = 0;
  virtual void SetBitmap(Bitmap&&) = 0;
};

using bmp_dialog_func =
  std::function<Optional<BitmapCommand*>(wxWindow&, DialogFeedback&)>;

using dialog_func =
  std::function<Optional<Command*>(wxWindow&, DialogFeedback&, Canvas&)>;

} // namespace

#endif
