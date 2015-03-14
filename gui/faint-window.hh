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

#ifndef FAINT_WINDOW_HH
#define FAINT_WINDOW_HH
#include <memory>
#include "app/app-context.hh"
#include "gui/entry-mode.hh"
#include "tools/tool-id.hh"
#include "util/id-types.hh"

namespace faint{

class ArtContainer;
class FaintWindowImpl;
class HelpFrame;
class InterpreterFrame;
class PythonContext;
class FilePath;

class FaintWindow{
public:
  FaintWindow(ArtContainer&,
    const PaintMap& palette,
    HelpFrame*,
    InterpreterFrame*,
    bool silentMode);
  ~FaintWindow();
  void Initialize();
  void AddFormat(Format*);
  void AddToPalette(const Paint&);
  void BeginTextEntry();
  void Close(bool force);
  void CloseDocument(Canvas&);
  void EndTextEntry();
  bool Exists(const CanvasId&) const;
  bool Focused() const;
  void FullScreen(bool enable);
  Canvas& GetActiveCanvas();
  Tool* GetActiveTool();
  AppContext& GetAppContext();
  Canvas& GetCanvas(const Index&);
  Index GetCanvasCount() const;
  std::vector<Format*>& GetFileFormats();
  Layer GetLayerType() const;
  PythonContext& GetPythonContext();
  wxFrame& GetRawFrame();

  void EnableToolbar(bool); // Fixme: Remove

  // Returns the currently shown settings, including overrides by
  // tools (especially the ObjSelectTool).
  Settings GetShownSettings() const;
  EntryMode GetTextEntryMode() const;

  // Returns the application's configured settings, ignoring overrides
  // by tools
  Settings& GetToolSettings();
  ToolId GetToolId() const;
  bool IsFullScreen() const;
  bool IsMaximized() const;
  void Maximize(bool);

  // Tells the FaintWindow that a modifier key was pressed or
  // released, for cursor refresh purposes.
  void ModifierKeyChange();
  Canvas& NewDocument(const ImageInfo&);
  void NextTab();
  void Open(const FileList&);
  Canvas* Open(const FilePath&, const change_tab&);
  void PreviousTab();
  void QueueLoad(const FileList& filenames);
  void Raise();
  bool Save(Canvas&);
  void SetActiveCanvas(const CanvasId&);
  void SelectLayer(Layer);
  void SelectTool(ToolId);
  void SetIcons(const wxIcon& icon16, const wxIcon& icon32);
  void SetPalette(const PaintMap&);
  void Show();
  bool ShowSaveAsDialog(Canvas&, bool backup=false);
  void ShowColorPanel(bool show);
  void ShowStatusbar(bool show);
  void ShowToolPanel(bool show);
  void ToggleMaximize();
  void Refresh();
  void UpdateShownSettings();
  void UpdateToolSettings(const Settings&);
private:
  std::unique_ptr<FaintWindowImpl> m_impl;
};

} // namespace

#endif
