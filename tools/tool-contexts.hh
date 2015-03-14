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

#ifndef FAINT_TOOL_CONTEXTS_HH
#define FAINT_TOOL_CONTEXTS_HH
#include "util/template-fwd.hh"

namespace faint{

class utf8_string;

class TextContext{
  // Context for text entry support.
public:
  virtual bool AcceptsPastedText() const = 0;
  virtual Optional<utf8_string> CopyText() const = 0;
  virtual Optional<utf8_string> CutText() = 0;
  virtual void Paste(const utf8_string&) = 0;
};

class HistoryContext{
  // Context for tool-specific undo/redo.
public:
  // Whether the tool currently allows redoing commands in the image.
  // Tools may need to prevent image-redo even if they currently have
  // nothing to redo. See \ref(allows-global-redo).
  virtual bool AllowsGlobalRedo() const = 0;

  virtual bool CanRedo() const = 0;
  virtual bool CanUndo() const = 0;
  virtual utf8_string GetRedoName() const = 0;
  virtual utf8_string GetUndoName() const = 0;
  bool PreventsGlobalRedo() const;
  virtual void Redo() = 0;
  virtual void Undo() = 0;
};

class SelectionContext{
  // Context for tool-specific selection.
public:
  virtual bool Delete() = 0;
  virtual bool Deselect() = 0;
  virtual bool SelectAll() = 0;
};

} // namespace

#endif
