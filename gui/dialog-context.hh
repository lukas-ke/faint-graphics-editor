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

#ifndef FAINT_DIALOG_CONTEXT_HH
#define FAINT_DIALOG_CONTEXT_HH
#include <memory>

class wxDialog;

namespace faint{

class CommandWindow;

class DialogContext{
public:
  int ShowModal(wxDialog&);

  // Shows a non-modal dialog
  virtual void Show(std::unique_ptr<CommandWindow>&&) = 0;
private:
  virtual void BeginModalDialog() = 0;
  virtual void EndModalDialog() = 0;
};

} // namespace

#endif
