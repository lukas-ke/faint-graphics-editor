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

#ifndef FAINT_HELP_FRAME_HH
#define FAINT_HELP_FRAME_HH
#include "util/dumb-ptr.hh"

class wxIcon;

namespace faint{

class ArtContainer;
class DirPath;

class HelpFrame {
// The frame containing the html help for Faint.
// Used instead of the wxHtmlHelpFrame to allow more customization,
// like closing on F1.
public:
  HelpFrame(const DirPath& rootDir, const ArtContainer&);
  ~HelpFrame();
  void Close();
  bool HasFocus() const;
  void Hide();
  bool IsIconized() const;
  bool IsShown() const;
  bool IsHidden() const;
  void Raise();
  void Restore();
  void SetIcons(const wxIcon& icon16, const wxIcon& icon32);
  void Show();
private:
  HelpFrame(const HelpFrame&);
  class HelpFrameImpl;
  dumb_ptr<HelpFrameImpl> m_impl;
};

} // namespace

#endif
