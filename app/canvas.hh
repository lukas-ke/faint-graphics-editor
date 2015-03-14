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

#ifndef FAINT_CANVAS_HH
#define FAINT_CANVAS_HH
#include <memory> // For weak_ptr
#include "commands/command-constants.hh"
#include "geo/primitive.hh"
#include "util/id-types.hh"
#include "util/objects.hh"
#include "util/status-interface.hh"
#include "util/template-fwd.hh"

namespace faint{

enum class Layer;
class Bitmap;
class Command;
class FilePath;
class Grid;
class Image;
class Index;
class IntPoint;
class IntSize;
class Object;
class Point;
class PosInfo;
class RasterSelection;
class Settings;
class ZoomLevel;

class ToolInterface{
  // Interface for triggering actions in a tool retrieved from a
  // Canvas.
  //
  // Requiring accesses to go through a ToolInterface allows the
  // Canvas to control additional actions, like refresh, which would
  // not be possible if the Tool-class was used directly.
public:
  virtual bool AcceptsPastedText() const = 0;

  virtual Optional<utf8_string> CopyText() const = 0;
  virtual Optional<utf8_string> CutText() = 0;

  // These functions return true if the tool complied.
  virtual bool Delete() = 0;
  virtual bool Deselect() = 0;
  virtual void Paste(const utf8_string&) = 0;

  virtual bool SelectAll() = 0;
  virtual bool SupportsSelection() const = 0;

  // Returns the layer type, taking the global layer type choice and
  // the tool in consideration.
  virtual Layer GetLayerType() const = 0;

  // Returns the current tool settings
  virtual Settings GetSettings() const = 0;
};

class ColorSpan;

class Canvas {
public:
  virtual ~Canvas() = default;
  virtual void CenterView(const Point&) = 0;
  virtual void ClearPointOverlay() = 0;
  virtual void CloseUndoBundle(const utf8_string&) = 0;
  virtual void DeselectObject(Object*) = 0;
  virtual void DeselectObjects() = 0;
  virtual void DeselectObjects(const objects_t&) = 0;
  virtual const Either<Bitmap, ColorSpan>& GetBackground() const = 0;
  virtual const Optional<Bitmap>& GetBitmap() const = 0;
  virtual Optional<FilePath> GetFilePath() const = 0;
  virtual const Image& GetFrame(const Index&) const = 0;
  virtual const Image& GetFrame(const FrameId&) const = 0;
  virtual Index GetFrameIndex(const Image&) const = 0;
  virtual Grid GetGrid() const = 0;
  virtual CanvasId GetId() const = 0;
  virtual const Image& GetImage() const = 0;
  virtual Point GetImageViewStart() const = 0;
  virtual IntPoint GetMaxScrollPos() = 0;
  virtual Index GetNumFrames() const = 0;
  virtual const objects_t& GetObjects() = 0;
  virtual const objects_t& GetObjectSelection() const = 0;
  virtual Optional<IntPoint> GetPointOverlay() const = 0;
  virtual PosInfo GetPosInfo(const IntPoint&) = 0;
  virtual const RasterSelection& GetRasterSelection() const = 0;
  virtual Point GetRelativeMousePos() = 0;
  virtual IntPoint GetScrollPos() = 0;
  virtual Index GetSelectedFrame() const = 0;
  virtual IntSize GetSize() const = 0;
  virtual ToolInterface& GetTool() = 0;
  virtual const ToolInterface& GetTool() const = 0;
  virtual coord GetZoom() const = 0;
  virtual ZoomLevel GetZoomLevel() const = 0;
  virtual bool Has(const FrameId&) const = 0;
  virtual bool Has(const ObjectId&) const = 0;
  virtual void NextFrame() = 0;
  virtual void NotifySaved(const FilePath&) = 0;
  virtual void OpenUndoBundle() = 0;
  virtual void PreviousFrame() = 0;
  virtual void Redo() = 0;
  virtual void Refresh() = 0;
  virtual void RunCommand(Command*) = 0;
  virtual void RunCommand(Command*, const FrameId&) = 0;
  virtual void RunDWIM() = 0;
  virtual void ScrollMaxDown() = 0;
  virtual void ScrollMaxLeft() = 0;
  virtual void ScrollMaxRight() = 0;
  virtual void ScrollMaxUp() = 0;
  virtual void ScrollPageDown() = 0;
  virtual void ScrollPageLeft() = 0;
  virtual void ScrollPageRight() = 0;
  virtual void ScrollPageUp() = 0;
  virtual void SelectFrame(const Index&) = 0;
  virtual void SelectObject(Object*, const deselect_old&) = 0;
  virtual void SelectObjects(const objects_t&, const deselect_old&) = 0;
  virtual void SetGrid(const Grid&) = 0;

  // Sets a temporary Bitmap which will be rendered instead of the
  // ordinary background. Allows tools and dialogs to modify a
  // bitmap they own, until the final changes are applied with a
  // command.
  virtual void SetMirage(const std::weak_ptr<Bitmap>&) = 0;

  // Sets a temporary RasterSelection which will be rendered instead
  // of the ordinary. Allows tools and dialogs to modify a selection
  // they own, until the final changes are applied
  // with a command.
  virtual void SetMirage(const std::weak_ptr<RasterSelection>&) = 0;

  virtual void SetPointOverlay(const IntPoint&) = 0;
  virtual void SetScrollPos(const IntPoint&) = 0;
  virtual void SetZoom(const ZoomLevel&) = 0;
  virtual void Undo() = 0;
  virtual void ZoomDefault() = 0;
  virtual void ZoomFit() = 0;
  virtual void ZoomIn() = 0;
  virtual void ZoomOut() = 0;
};

} // namespace

#endif
