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

#ifndef FAINT_APP_CONTEXT_HH
#define FAINT_APP_CONTEXT_HH
#include <vector>
#include <functional>
#include "gui/command-dialog.hh" // For dialog_func
#include "gui/dialog-context.hh"
#include "rendering/extra-overlay.hh"
#include "util/distinct.hh"
#include "util/id-types.hh"
#include "util/settings.hh"

namespace faint{

enum class Layer;
enum class ToolId;
class AppContext;
class Canvas;
class FileList;
class FilePath;
class Format;
class Grid;
class ImageInfo;
class ImageProps;
class Index;
class PaintMap;
class PosInfo;
class ResizeDialogOptions;
class Tool;
class TransparencyStyle;
class category_app_context;
using change_tab = Distinct<bool, category_app_context, 0>;

class Interaction{
  // Fixme: Extend. Consider using as forward for tools too.
public:
  virtual bool MouseMove(const PosInfo&) = 0;
};

class AppContext {
public:
  virtual ~AppContext() = default;
  virtual void AddFormat(Format*) = 0;
  virtual void AddToPalette(const Paint&) = 0;
  virtual void BeginModalDialog() = 0;
  virtual void BeginTextEntry() = 0;
  virtual void Close(Canvas&) = 0;
  virtual void DialogOpenFile() = 0;
  virtual void DialogSaveAs(Canvas&, bool backup) = 0;
  virtual void EndModalDialog() = 0;
  virtual void EndTextEntry() = 0;
  virtual bool Exists(const CanvasId&) = 0;
  virtual bool FaintWindowFocused() const = 0;
  virtual BoolSetting::ValueType Get(const BoolSetting&) = 0;
  virtual StringSetting::ValueType Get(const StringSetting&) = 0;
  virtual IntSetting::ValueType Get(const IntSetting&) = 0;
  virtual ColorSetting::ValueType Get(const ColorSetting&) = 0;
  virtual FloatSetting::ValueType Get(const FloatSetting&) = 0;
  virtual Canvas& GetActiveCanvas() = 0;
  virtual Tool* GetActiveTool() = 0;
  virtual Canvas& GetCanvas(const Index&) = 0;
  virtual int TabletGetCursor() = 0;
  virtual Index GetCanvasCount() const = 0;
  virtual Grid GetDefaultGrid() const = 0;
  virtual ImageInfo GetDefaultImageInfo() = 0;
  virtual ResizeDialogOptions GetDefaultResizeDialogOptions() const = 0;
  virtual DialogContext& GetDialogContext() = 0;
  virtual std::vector<Format*> GetFormats() = 0;
  virtual Layer GetLayerType() const = 0;
  virtual IntPoint GetMousePos() = 0;
  virtual ToolId GetToolId() const = 0;
  virtual Settings GetToolSettings() const = 0;
  virtual const TransparencyStyle& GetTransparencyStyle() const = 0;
  virtual bool IsFullScreen() const = 0;
  virtual Canvas* Load(const FilePath&, const change_tab&) = 0;
  virtual void Load(const FileList&) = 0;
  virtual Canvas* LoadAsFrames(const FileList&, const change_tab&) = 0;
  virtual void Maximize() = 0;
  virtual void MaximizeInterpreter() = 0;
  virtual bool ModalDialogShown() const = 0;
  virtual Canvas& NewDocument(const ImageInfo&) = 0;
  virtual Canvas& NewDocument(ImageProps&&) = 0;
  virtual void NextTab() = 0;
  virtual void PreviousTab() = 0;

  // Queues the files for loading, but returns immediately
  virtual void QueueLoad(const FileList&) = 0;

  virtual void Quit() = 0;
  virtual void RaiseWindow() = 0;
  virtual bool Save(Canvas&) = 0;
  virtual void SelectTool(ToolId) = 0;
  virtual void Set(const BoolSetting&, BoolSetting::ValueType) = 0;
  virtual void Set(const StringSetting&, const StringSetting::ValueType&) = 0;
  virtual void Set(const IntSetting&, IntSetting::ValueType) = 0;
  virtual void Set(const ColorSetting&, ColorSetting::ValueType) = 0;
  virtual void Set(const FloatSetting&, FloatSetting::ValueType) = 0;
  virtual void SetActiveCanvas(const CanvasId&) = 0;
  virtual void SetDefaultGrid(const Grid&) = 0;
  virtual void SetDefaultResizeDialogOptions(const ResizeDialogOptions&) = 0;
  virtual void SetInterpreterBackground(const ColRGB&) = 0;
  virtual void SetInterpreterTextColor(const ColRGB&) = 0;
  virtual void SetLayer(Layer) = 0;
  virtual void SetPalette(const PaintMap&) = 0;
  virtual void SetTransparencyStyle(const TransparencyStyle&) = 0;
  virtual void ModalFull(const dialog_func&) = 0;
  virtual void Modal(const bmp_dialog_func&) = 0;
  virtual void ShowColorPanel(bool) = 0;
  virtual void ShowPythonConsole() = 0;
  virtual void ShowStatusbar(bool) = 0;
  virtual void ShowToolPanel(bool) = 0;
  virtual void ToggleFullScreen(bool) = 0;
  virtual void ToggleHelpFrame() = 0;
  virtual void ToggleMaximize() = 0;
  virtual void TogglePythonConsole() = 0;

  // Shows the settings and values from the active tool in the UI
  // setting controls
  virtual void UpdateShownSettings() = 0;

  // Makes the given settings the application-wide settings
  virtual void UpdateToolSettings(const Settings&) = 0;
  virtual Interaction& GetInteraction() = 0;
  virtual ExtraOverlay& GetExtraOverlay() = 0; // Fixme: Rename/remove
};

} // namespace

#endif
