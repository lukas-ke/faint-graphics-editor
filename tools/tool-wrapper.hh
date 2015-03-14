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

#ifndef FAINT_TOOL_WRAPPER_HH
#define FAINT_TOOL_WRAPPER_HH
#include "tools/tool-id.hh"

namespace faint{

class AppContext;
class ToolWrapper{
  // Provides access to the active tool (shared between all canvases),
  // or a canvas-specific tool, for example the text editing which
  // replaces object selection when a text object is double-clicked.
public:
  ToolWrapper(AppContext&);
  Tool& GetTool();
  const Tool& GetTool() const;
  ToolId GetToolId() const;

  // See Tool::DrawBeforeZoom
  bool DrawBeforeZoom() const;

  // Sets a switched-in tool which replaces the regular tool for
  // getters until ClearSwitched is called. The switched tool will
  // also be cleared if the regular tool is changed externally.
  void SetSwitched(Tool*);
  void ClearSwitched();

  ToolWrapper& operator=(const ToolWrapper&) = delete;
private:
  AppContext& m_app;
  mutable Tool* m_lastSeen;
  mutable Tool* m_switched;
};

} // namespace

#endif
