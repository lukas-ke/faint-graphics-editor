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

#ifndef FAINT_FRAME_CTRL_HH
#define FAINT_FRAME_CTRL_HH

namespace faint{

class ArtContainer;
class StatusInterface;

class AppContext;

class FrameCtrl{
  // Control for selecting and reordering animation frames (and such).
public:
  FrameCtrl(wxWindow*, AppContext&, StatusInterface&, const ArtContainer&);
  ~FrameCtrl();

  wxWindow* AsWindow();

  // Update the number of frames shown in the control. Returns true if
  // changed, to allow parents to accomodate changed size
  // requirements.
  bool Update();

  FrameCtrl(const FrameCtrl&) = delete;
private:
  class FrameCtrlImpl;
  FrameCtrlImpl* m_impl;
};

} // namespace
#endif
