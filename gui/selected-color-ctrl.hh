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

#ifndef FAINT_SELECTED_COLOR_CTRL_HH
#define FAINT_SELECTED_COLOR_CTRL_HH

class wxWindow;

namespace faint{

class ColorChoice;
class DialogContext;
class IntSize;
class StatusInterface;

class SelectedColorCtrlImpl;

class SelectedColorCtrl{
public:
  SelectedColorCtrl(wxWindow* parent,
    const IntSize&,
    StatusInterface&,
    DialogContext&);
  ~SelectedColorCtrl();
  wxWindow* AsWindow();
  void UpdateColors(const ColorChoice&);
  enum class Which{HIT_FG, HIT_BG, HIT_NEITHER};

  SelectedColorCtrl(const SelectedColorCtrl&) = delete;
  SelectedColorCtrl& operator=(const SelectedColorCtrl&) = delete;
private:
  SelectedColorCtrlImpl* m_impl;
};

} // namespace

#endif
