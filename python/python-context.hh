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

#ifndef FAINT_PYTHON_CONTEXT_HH
#define FAINT_PYTHON_CONTEXT_HH
#include "util/template-fwd.hh"
#include "util/id-types.hh"

namespace faint{

class Canvas;
class Command;
class KeyPress;
class utf8_string;

class category_python_context;
using bind_global = Distinct<bool, category_python_context, 0>;

class PythonContext{
public:
  virtual ~PythonContext() = default;
  virtual void Bind(const KeyPress&, const bind_global&) = 0;
  virtual bool Bound(const KeyPress&) const = 0;
  virtual bool BoundGlobal(const KeyPress&) const = 0;

  // Adds ... to the interpreter.
  virtual void Continuation() = 0;

  // Signal that a read-eval-print or function evaluation in python is
  // complete, e.g. to perform refresh etc. after batched drawing.
  virtual void EvalDone() = 0;

  // Tell the interpreter to get a key from the user.
  virtual void GetKey() = 0;

  // Gets the name to be used for the next Python command bundle if
  // set, empty string otherwise.
  virtual utf8_string GetCommandName(const Canvas*) = 0;

  // Sets the name to be used for the next Python command bundle. This
  // will be cleared on EvalDone().
  virtual void SetCommandName(const Canvas*, const utf8_string&) = 0;

  // Interface printing.
  virtual void IntFaintPrint(const utf8_string&) = 0;

  // Add a new prompt-line to the interpreter.
  virtual void NewPrompt() = 0;

  // Prints to interpreter console, typically for debugging.
  virtual void Print(const utf8_string&) = 0;

  // Flag the canvas as requiring a refresh, queuing the refresh until
  // EvalDone().
  //
  // The flagging is done automatically by RunCommand, so this
  // should be used for actions that do not involve commands, like
  // object selection.
  virtual void QueueRefresh(Canvas*) = 0;

  // Run a Faint-command from a Python interface function.
  virtual void RunCommand(Canvas*, Command*) = 0;

  // Run a Faint-command from a Python interface function, targetting
  // a specific frame.
  virtual void RunCommand(Canvas*, Command*, const FrameId&) = 0;

  virtual void Unbind(const KeyPress&) = 0;
};

} // namespace

#endif
