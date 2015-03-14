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

#ifndef FAINT_TOOL_HH
#define FAINT_TOOL_HH
#include "app/resource-id.hh" // For Cursor
#include "tools/refresh-info.hh"
#include "tools/tool-contexts.hh"
#include "tools/tool-id.hh"
#include "util/template-fwd.hh"

namespace faint{

class Command;
class FaintDC;
class IntRect;
class Point;
class Overlays;
class utf8_string;

enum class ToolResult{
  // Return values for Tool methods.
  NONE, // Action had no externally relevant effect
  DRAW, // Tool wants to draw
  SETTING_CHANGED, // A setting was changed
  COMMIT, // Tool has a command ready
  CHANGE, // Tool wishes to uhm switch to a new tool somehow
  CANCEL // Tool aborted
};

class KeyInfo;
class PosInfo;
class BoundSetting;

class Tool {
public:
  explicit Tool(ToolId);
  virtual ~Tool() = default;
  virtual ToolResult Char(const KeyInfo&) = 0;
  virtual ToolResult DoubleClick(const PosInfo&) = 0;
  virtual void Draw(FaintDC&, Overlays&, const PosInfo&) = 0;

  // True if the tool should be rendered before scaling for zoom.
  // Typically true for raster graphics, see \ref(draw-before-zoom).
  virtual bool DrawBeforeZoom(Layer) const = 0;

  // True if the tool monopolizes tool-setting changes, or if it
  // allows changes to the common tool settings. In practice, probably
  // only used for making object selection setting changes affect the
  // objects, and leave the general tool settings unchanged.
  virtual bool EatsSettings() const = 0;
  virtual Command* GetCommand() = 0;
  virtual Cursor GetCursor(const PosInfo&) const = 0;
  ToolId GetId() const;

  // Returns the (in image coordinates)-rectangle the tool has
  // modified.
  virtual IntRect GetRefreshRect(const RefreshInfo&) const = 0;

  virtual const Settings& GetSettings() const = 0;

  Optional<faint::HistoryContext&> HistoryContext();
  virtual Optional<const faint::HistoryContext&> HistoryContext() const = 0;

  virtual ToolResult MouseDown(const PosInfo&) = 0;
  virtual ToolResult MouseUp(const PosInfo&) = 0;
  virtual ToolResult MouseMove(const PosInfo&) = 0;

  // The tool is given a chance to commit before being replaced
  virtual ToolResult Preempt(const PosInfo&) = 0;

  // Whether the tool requires a refresh when pointer leaves drawing
  // area. Not called during mouse capture, i.e. while mouse held.
  virtual bool RefreshOnMouseOut() const = 0;

  // Called to notify that the selection changed outside the tool's
  // control.
  virtual void SelectionChange() = 0;

  Optional<faint::SelectionContext&> SelectionContext();
  virtual Optional<const faint::SelectionContext&> SelectionContext() const = 0;

  virtual bool Set(const BoundSetting&) = 0;
  virtual void SetLayer(Layer) = 0;

  // Call to update the settings in the tool. Returns true if any
  // setting used by the tool was changed.
  virtual bool UpdateSettings(const Settings&) = 0;

  // Implemented in terms of the const-version.
  Optional<faint::TextContext&> TextContext();
  virtual Optional<const faint::TextContext&> TextContext() const = 0;

  Tool& operator=(const Tool&) = delete;
private:
  ToolId m_id;
};

} // namespace

#endif
