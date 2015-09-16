// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
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

#ifndef FAINT_WINDOW_APP_CONTEXT_HH
#define FAINT_WINDOW_APP_CONTEXT_HH
#include "app/app-context.hh"
#include "app/faint-common-cursors.hh"
#include "app/faint-slider-cursors.hh"
#include "gui/command-window.hh"
#include "gui/dialogs/resize-dialog-options.hh"
#include "gui/faint-window.hh"
#include "gui/transparency-style.hh"
#include "tools/tool.hh"
#include "util/bound-setting.hh" // Fixme: For BoundSetting (change_setting)
#include "util/dumb-ptr.hh"
#include "util/grid.hh"

class wxStatusBar;

namespace faint{

class Art;
class Canvas;
class FilePath;
class FaintWindow;
class HelpFrame;
class InterpreterFrame;
class TabCtrl;

class SBInterface final : public StatusInterface {
public:
  SBInterface(wxStatusBar&);
  void SetMainText(const utf8_string& text) override;
  void SetText(const utf8_string& text, int field=0) override;
  void Clear() override;

  SBInterface& operator=(const SBInterface&) = delete;
private:
  wxStatusBar& m_statusbar;
};

using from_control = LessDistinct<bool, 0>;

class FaintDialogContext final : public DialogContext{
public:
  FaintDialogContext(AppContext&, const Art&, FaintWindow&);

  SliderCursors& GetSliderCursors() override;
  CommonCursors& GetCommonCursors() override;
  void Show(std::unique_ptr<CommandWindow>&& w) override;
  void UpdateSettings(const Settings&);
  void Reinitialize();
  Optional<CommandWindow&> ShownWindow();
  void Close();

private:
  void BeginModalDialog() override;
  void EndModalDialog() override;
  void OnClosed(BitmapCommand*);

  AppContext& m_app;
  std::unique_ptr<CommandWindow> m_commandWindow;
  FaintCommonCursors m_commonCursors;
  FaintWindow& m_faintWindow;
  FaintSliderCursors m_sliderCursors;
  std::unique_ptr<WindowFeedback> m_windowFeedback;
};

class FaintWindowExtraOverlay final : public ExtraOverlay{
public:
  // Fixme: Weird class. Use the WindowFeedback instead?
  FaintWindowExtraOverlay(FaintDialogContext&);
  void Draw(FaintDC& dc, Overlays& overlays, const PosInfo& info) override;

  FaintWindowExtraOverlay& operator=(FaintWindowExtraOverlay&) = delete;
private:
  FaintDialogContext& m_dialogContext;
};

class FaintWindowInteraction final : public Interaction{
public:
  FaintWindowInteraction(FaintDialogContext&);
  bool MouseMove(const PosInfo&) override;

  FaintWindowInteraction& operator=(const FaintWindowInteraction&) = delete;
private:
  FaintDialogContext& m_ctx;
};

class FaintWindowContext final : public AppContext {
public:
  FaintWindowContext(FaintWindow&, const Art&,
    wxStatusBar&, HelpFrame&, InterpreterFrame&);
  void AddFormat(Format*) override;
  void AddToPalette(const Paint&) override;
  void BeginModalDialog() override;
  void BeginTextEntry() override;
  void Close(Canvas& canvas) override;
  void DialogOpenFile() override;
  void DialogSaveAs(Canvas& canvas, bool backup) override;
  void EndModalDialog() override;
  void EndTextEntry() override;
  bool Exists(const CanvasId&) override;
  bool FaintWindowFocused() const override;
  BoolSetting::ValueType Get(const BoolSetting&) override;
  StringSetting::ValueType Get(const StringSetting&) override;
  IntSetting::ValueType Get(const IntSetting&) override;
  PaintSetting::ValueType Get(const PaintSetting&) override;
  FloatSetting::ValueType Get(const FloatSetting&) override;
  Interaction& GetInteraction() override;
  ExtraOverlay& GetExtraOverlay() override;
  Canvas& GetActiveCanvas() override;
  Tool* GetActiveTool() override;
  Canvas& GetCanvas(const Index&) override;
  Canvas& GetCanvas(const CanvasId&) override;
  Index GetCanvasCount() const override;
  Grid GetDefaultGrid() const override;
  ImageInfo GetDefaultImageInfo() override;
  FaintDialogContext& GetDialogContext() override;
  std::vector<Format*> GetFormats() override;
  ResizeDialogOptions GetDefaultResizeDialogOptions() const override;
  Layer GetLayerType() const override;
  IntPoint GetMousePos() override;
  StatusInterface& GetStatusInfo(); // Non virtual
  ToolId GetToolId() const override;
  Settings GetToolSettings() const override;
  const TransparencyStyle& GetTransparencyStyle() const override;
  bool IsFullScreen() const override;
  Canvas* Load(const FilePath&, const change_tab&) override;
  void Load(const FileList&) override;
  Canvas* LoadAsFrames(const FileList& paths,
    const change_tab& changeTab) override;
  void Maximize() override;
  void MaximizeInterpreter() override;
  bool ModalDialogShown() const override;
  Canvas& NewDocument(const ImageInfo& info) override;
  Canvas& NewDocument(ImageProps&& props) override;
  void NextTab() override;
  void PreviousTab() override;
  void QueueLoad(const FileList& filenames) override;
  void Quit(bool) override;
  void RaiseWindow() override;
  bool Save(Canvas& canvas) override;
  void SelectTool(ToolId id) override;
  void Set(const BoolSetting&, BoolSetting::ValueType) override;
  void Set(const StringSetting&, const StringSetting::ValueType&) override;
  void Set(const IntSetting&, IntSetting::ValueType) override;
  void Set(const PaintSetting&, PaintSetting::ValueType) override;
  void Set(const FloatSetting&, FloatSetting::ValueType) override;
  void SetActiveCanvas(const CanvasId&) override;
  void SetDefaultGrid(const Grid&) override;
  void SetDefaultResizeDialogOptions(const ResizeDialogOptions& opts) override;
  void SetInterpreterBackground(const ColRGB& c) override;
  void SetInterpreterTextColor(const ColRGB& c) override;
  void SetPalette(const PaintMap& paintMap) override;
  void SetTransparencyStyle(const TransparencyStyle& style) override;
  void SetLayer(Layer layer) override;
  void ModalFull(const dialog_func& show_dialog) override;
  void Modal(const bmp_dialog_func& show_dialog) override;
  void ToggleHelpFrame() override;
  void TogglePythonConsole() override;
  void ShowColorPanel(bool show) override;
  void ShowPythonConsole() override;
  void ShowStatusbar(bool show) override;
  void ShowToolPanel(bool show) override;
  int TabletGetCursor() override;

  // Note: Not an override, used directly by FaintWindow
  void TabletSetCursor(int tabletCursor);

  void ToggleFullScreen(bool) override;
  void ToggleMaximize() override;
  void UpdateShownSettings() override;
  void UpdateToolSettings(const Settings&) override;
  void SetTabCtrl(TabCtrl*); // Non virtual

private:
  FaintDialogContext m_dialogContext;
  FaintWindowExtraOverlay m_extraOverlay;
  FaintWindow& m_faintWindow;
  HelpFrame& m_helpFrame;
  FaintWindowInteraction m_interaction;
  InterpreterFrame& m_interpreterFrame;
  int m_modalDialog;
  SBInterface m_statusbar;
  Grid m_defaultGrid;
  ResizeDialogOptions m_defaultResizeSettings;
  TransparencyStyle m_transparencyStyle;
  int m_tabletCursor;
  dumb_ptr<TabCtrl> m_tabControl;
};

template<typename T>
void change_setting(FaintWindow& window,
  const T& setting,
  typename T::ValueType value,
  const from_control& fromControl,
  FaintDialogContext& dialogContext)
{
  dialogContext.ShownWindow().Visit(
    [&](CommandWindow& w){
      const auto& windowSettings(w.GetSettings());
      if (windowSettings.Has(setting)){
        // Fixme: Silly
        dialogContext.UpdateSettings(with(windowSettings, setting, value));
        // w.UpdateSettings(with(windowSettings, setting, value));
      }
    },
    [](){});

  Tool* tool = window.GetActiveTool();
  if (tool->GetSettings().Has(setting)){
    bool toolModified = tool->Set({setting, value});
    if (toolModified) {
      window.GetActiveCanvas().Refresh();
    }
    if (tool->EatsSettings()){
      if (!fromControl.Get()){
        window.UpdateShownSettings();
      }
      return;
    }
  }
  window.GetToolSettings().Set(setting, value);
  if (!fromControl.Get()){
    window.UpdateShownSettings();
  }
}

} // namespace

#endif
