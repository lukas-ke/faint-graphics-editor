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

#ifndef FAINT_TOOL_BAR_HH
#define FAINT_TOOL_BAR_HH
#include "tools/tool-id.hh"
#include "util/dumb-ptr.hh"

class wxWindow;

namespace faint{

class ArtContainer;
class StatusInterface;

class Toolbar{
public:
  Toolbar(wxWindow*, StatusInterface&, ArtContainer&);
  wxWindow* AsWindow();
  void SendLayerChoiceEvent(Layer);
  void SendToolChoiceEvent(ToolId);
private:
  class ToolbarImpl;
  dumb_ptr<ToolbarImpl> m_impl;
};

} // namespace

#endif
