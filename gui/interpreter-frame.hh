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

#ifndef FAINT_INTERPRETER_FRAME_HH
#define FAINT_INTERPRETER_FRAME_HH
#include "text/utf8-string.hh"
#include "util/dumb-ptr.hh"

class wxIcon;

namespace faint{

class ColRGB;
class InterpreterCtrl;
class InterpreterFrameImpl;

class InterpreterFrame{
public:
  InterpreterFrame();
  ~InterpreterFrame();
  void AddNames(const std::vector<utf8_string>&);
  void Close();
  bool HasFocus() const;
  void Hide();
  void SetBackgroundColor(const ColRGB&);
  void SetIcons(const wxIcon& icon16, const wxIcon& icon32);
  void GetKey();
  void IntFaintPrint(const utf8_string&);
  bool IsIconized() const;
  bool IsHidden() const;
  bool IsMaximized() const;
  void Maximize(bool);
  void NewContinuation();
  void NewPrompt();
  void Print(const utf8_string&);
  void Raise();
  void Restore();
  void SetTextColor(const ColRGB&);
  void Show();
private:
  dumb_ptr<InterpreterFrameImpl> m_impl;
};

} // namespace

#endif
