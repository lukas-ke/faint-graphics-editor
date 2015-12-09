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

#include <memory>
#include "wx/frame.h"
#include "wx/statusbr.h"
#include "app/canvas.hh" // For context_targetted
#include "app/context-commands.hh" // For context_targetted
#include "app/faint-window-app-context.hh"
#include "bitmap/draw.hh"
#include "gui/art.hh"
#include "gui/help-frame.hh"
#include "gui/interpreter-frame.hh"
#include "gui/tab-ctrl.hh" // Fixme: Remove
#include "rendering/extra-overlay.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/file-format-util.hh"
#include "util-wx/gui-util.hh"
#include "util/image-props.hh"
#include "util/image.hh"
#include "util/pos-info-constants.hh"
#include "util/visit-selection.hh"

namespace faint{

static Optional<DirPath> get_canvas_dir(const Canvas& canvas){
  Optional<FilePath> oldFileName(canvas.GetFilePath());
  return oldFileName.IsSet() ?
    option(oldFileName.Get().StripFileName()) :
    no_option();
}

SBInterface::SBInterface(wxStatusBar& statusbar)
  : m_statusbar(statusbar)
{}

void SBInterface::SetMainText(const utf8_string& text){
  m_statusbar.SetStatusText(to_wx(text), 0);
}

void SBInterface::SetText(const utf8_string& text, int field){
  assert(field < m_statusbar.GetFieldsCount());
  m_statusbar.SetStatusText(to_wx(text), field + 1);
}

void SBInterface::Clear(){
  for (int i = 0; i != m_statusbar.GetFieldsCount(); i++){
    m_statusbar.SetStatusText("", i);
  }
}

template<typename T>
class WindowFeedbackImpl : public WindowFeedback{
public:
  WindowFeedbackImpl(AppContext& app, const T& onClose) :
    m_app(app),
    m_onClose(onClose)
  {}

  bool FloatingSelection() const override{
    return m_rasterSelection != nullptr && m_reallyFloating;
  }

  void Closed() override{
    m_onClose(nullptr);
  }

  void Closed(BitmapCommand* cmd) override{
    m_onClose(cmd);
  }

  void Reinitialize() override{
    // Called e.g. after Canvas-change so that
    // dialogs can fetch the new bitmap
    Reset();
    Initialize();
  }

  Bitmap GetBitmap() override{
    if (m_bitmap == nullptr){
      Initialize();
    }
    return *m_bitmap;
  }

  void SetPreview(const Bitmap& bitmap) override{
    if (m_bitmap == nullptr){
      m_bitmap  = std::make_shared<Bitmap>(bitmap);
    }
    else{
      *m_bitmap = bitmap;
    }
    Update();
  }

  void Reset() override{
    m_bitmap = nullptr;
    m_rasterSelection = nullptr;
  }

  void UpdateSettings(const Settings& s) override{
    if (m_rasterSelection != nullptr){
      m_rasterSelection->SetOptions(raster_selection_options(s));
      Update();
    }
  }

private:
  void Update(){
    auto& canvas = m_app.GetActiveCanvas();
    if (m_rasterSelection != nullptr && m_rasterSelection->Floating()){
      m_rasterSelection->SetFloatingBitmap(*m_bitmap,
        m_rasterSelection->TopLeft());
      canvas.SetMirage(m_rasterSelection);
    }
    else{
      canvas.SetMirage(m_bitmap);
    }
  }

  void Initialize(){
    const auto& c = m_app.GetActiveCanvas();
    const Image& active = c.GetImage();
    const RasterSelection& selection = active.GetRasterSelection();
    sel::visit(selection,
      [&](const sel::Empty&){
        // Fixme: Handle possible OOM ..or I dunno, use some fake bitmap
        active.GetBackground().Visit(
          [&](const Bitmap& bmp){
            m_bitmap = std::make_shared<Bitmap>(bmp);
          },
          [&](const ColorSpan& span){
            m_bitmap = std::make_shared<Bitmap>(span.size,
              span.color);
          });
        m_reallyFloating = false;
      },
      [&](const sel::Rectangle& r){
        // Create a floating selection mirage from the non-floating
        // selection
        active.GetBackground().Visit(
          [&](const Bitmap& bmp){
            m_bitmap = std::make_shared<Bitmap>(subbitmap(bmp,
              r.Rect()));
            m_rasterSelection = std::make_shared<RasterSelection>();
            m_rasterSelection->SetState(SelectionState(*m_bitmap,
              selection.TopLeft()));
          },
          [&](const ColorSpan& span){
            m_bitmap = std::make_shared<Bitmap>(
              r.Rect().GetSize(), span.color);
            m_rasterSelection = std::make_shared<RasterSelection>();
            m_rasterSelection->SetState(SelectionState(*m_bitmap,
              r.TopLeft()));
          });
        m_reallyFloating = false;
      },
      [&](const sel::Floating& f){
        m_rasterSelection = std::make_shared<RasterSelection>(selection);
        m_bitmap = std::make_shared<Bitmap>(f.GetBitmap());
        m_reallyFloating = true;
      });
  }

  AppContext& m_app;
  std::shared_ptr<Bitmap> m_bitmap;
  T m_onClose;
  std::shared_ptr<RasterSelection> m_rasterSelection;
  bool m_reallyFloating;
};

template<typename T>
std::unique_ptr<WindowFeedbackImpl<T>> create_window_feedback(AppContext& app,
  const T& onClose)
{
  return std::make_unique<WindowFeedbackImpl<T>>(app, onClose);
}

FaintDialogContext::FaintDialogContext(AppContext& app,
  const Art& art,
  FaintWindow& faintWindow)
  : m_app(app),
    m_commonCursors(art.Get(Cursor::BLANK), art.Get(Cursor::CROSSHAIR)),
    m_faintWindow(faintWindow),
    m_sliderCursors(art.Get(Cursor::HORIZONTAL_SLIDER),
      art.Get(Cursor::VERTICAL_SLIDER),
      art.Get(Cursor::RESIZE_WE),
      art.Get(Cursor::RESIZE_NS))
{
  m_windowFeedback = std::move(create_window_feedback(app,
    [this](BitmapCommand* cmd){
      OnClosed(cmd);
    }));
}

SliderCursors& FaintDialogContext::GetSliderCursors(){
  return m_sliderCursors;
}

CommonCursors& FaintDialogContext::GetCommonCursors(){
  return m_commonCursors;
}

void FaintDialogContext::Show(std::unique_ptr<CommandWindow>&& w){
  assert(w != nullptr);
  m_app.BeginModalDialog(); // Fixme
  m_commandWindow = std::move(w);
  m_commandWindow->Show(m_faintWindow.GetRawFrame(),
    m_app.GetToolSettings(),
    *m_windowFeedback);
}

void FaintDialogContext::UpdateSettings(const Settings& s){
  ShownWindow().IfSet(
    [&](CommandWindow& w){
      m_windowFeedback->UpdateSettings(s);
      w.UpdateSettings(s);
    });
}

void FaintDialogContext::Reinitialize(){
  ShownWindow().IfSet(
    [&](CommandWindow& w){
      m_windowFeedback->Reinitialize();
      w.Reinitialize(*m_windowFeedback);
    });
}

Optional<CommandWindow&> FaintDialogContext::ShownWindow(){
  if (m_commandWindow == nullptr || !m_commandWindow->IsShown()){
    return {};
  }
  return Optional<CommandWindow&>(*m_commandWindow);
}

void FaintDialogContext::Close(){
  m_commandWindow.reset(nullptr); // Fixme: Called only on destroy? Will I get OnClosed? <- Fixme: Need to use Destroy. <- No, not if CommandWindow deletes properly
}

void FaintDialogContext::BeginModalDialog(){
  m_app.BeginModalDialog();
}

void FaintDialogContext::EndModalDialog(){
  m_app.EndModalDialog();
}

void FaintDialogContext::OnClosed(BitmapCommand* cmd){
  auto& canvas = m_app.GetActiveCanvas();
  if (cmd != nullptr){
    canvas.RunCommand(context_targetted(cmd, canvas));

  }
  m_app.EndModalDialog(); // Fixme
  m_windowFeedback->Reset();
  m_app.GetActiveCanvas().Refresh();
  canvas.Refresh();
  // m_faintWindow.EnableToolbar(true);
}

class DialogFeedbackImpl : public DialogFeedback{
  // Fixme: Remove in favor of WindowFeedbackImpl
public:
  explicit DialogFeedbackImpl(Canvas& canvas)
    : m_canvas(canvas)
  {}

  const Bitmap& GetBitmap() override{
    if (m_bitmap == nullptr){
      Initialize();
    }
    return *m_bitmap;
  }

  void SetBitmap(const Bitmap& bmp) override{
    if (m_bitmap == nullptr){
      m_bitmap = std::make_shared<Bitmap>(bmp);
    }
    else{
      *m_bitmap = bmp;
    }
    Update();
  }

  void SetBitmap(Bitmap&& bmp) override{
    if (m_bitmap == nullptr){
      m_bitmap = std::make_shared<Bitmap>(std::move(bmp));
    }
    else{
      *m_bitmap = std::move(bmp);
    }
    Update();
  }

private:
  void Update(){
    if (m_rasterSelection != nullptr && m_rasterSelection->Floating()){
      m_rasterSelection->SetFloatingBitmap(*m_bitmap,
        m_rasterSelection->TopLeft());
      m_canvas.SetMirage(m_rasterSelection);
    }
    else{
      m_canvas.SetMirage(m_bitmap);
    }
  }

  void Initialize(){
    const Image& active = m_canvas.GetImage();
    const RasterSelection& selection = active.GetRasterSelection();
    // Fixme: Duplicates WindowFeedbackImpl
    sel::visit(selection,
      [&](const sel::Empty&){
        // Fixme: Handle possible OOM ..or I dunno, use some fake bitmap
        active.GetBackground().Visit(
          [&](const Bitmap& bmp){
            m_bitmap = std::make_shared<Bitmap>(bmp);
          },
          [&](const ColorSpan& span){
            m_bitmap = std::make_shared<Bitmap>(span.size,
              span.color);
          });
      },
      [&](const sel::Rectangle& r){
        // Create a floating selection mirage from the non-floating
        // selection
        active.GetBackground().Visit(
          [&](const Bitmap& bmp){
            m_bitmap = std::make_shared<Bitmap>(subbitmap(bmp,
              r.Rect()));
            m_rasterSelection = std::make_shared<RasterSelection>();
            m_rasterSelection->SetState(SelectionState(*m_bitmap,
              selection.TopLeft()));
          },
          [&](const ColorSpan& span){
            m_bitmap = std::make_shared<Bitmap>(
              r.Rect().GetSize(), span.color);
            m_rasterSelection = std::make_shared<RasterSelection>();
            m_rasterSelection->SetState(SelectionState(*m_bitmap,
              r.TopLeft()));
          });
      },
      [&](const sel::Floating& f){
        m_rasterSelection = std::make_shared<RasterSelection>(selection);
        m_bitmap = std::make_shared<Bitmap>(f.GetBitmap());
      });
  }

  std::shared_ptr<Bitmap> m_bitmap;
  Canvas& m_canvas;
  std::shared_ptr<RasterSelection> m_rasterSelection;
};

FaintWindowInteraction::FaintWindowInteraction(FaintDialogContext& ctx)
  : m_ctx(ctx)
{}

bool FaintWindowInteraction::MouseMove(const PosInfo& posInfo){
  return m_ctx.ShownWindow().Visit(
    [&posInfo](CommandWindow& w){
      return w.MouseMove(posInfo);
    },
    [](){
      return false;
    });
}

FaintWindowExtraOverlay::FaintWindowExtraOverlay(FaintDialogContext& ctx) :
  m_dialogContext(ctx)
{}

void FaintWindowExtraOverlay::Draw(FaintDC& dc,
  Overlays& overlays,
  const PosInfo& info)
{
    m_dialogContext.ShownWindow().IfSet(
      [&](CommandWindow& w){
        w.Draw(dc, overlays, info);
      });
}


FaintWindowContext::FaintWindowContext(FaintWindow& window,
  const Art& art,
  wxStatusBar& statusbar,
  HelpFrame& helpFrame,
  InterpreterFrame& interpreterFrame)
  : m_dialogContext(*this, art, window),
    m_extraOverlay(m_dialogContext),
    m_faintWindow(window),
    m_helpFrame(helpFrame),
    m_interaction(m_dialogContext),
    m_interpreterFrame(interpreterFrame),
    m_modalDialog(0),
    m_statusbar(statusbar),
    m_tabletCursor(TABLET_CURSOR_PUCK)
{}

void FaintWindowContext::AddFormat(Format* fileFormat){
  m_faintWindow.AddFormat(fileFormat);
}

void FaintWindowContext::AddToPalette(const Paint& paint){
  m_faintWindow.AddToPalette(paint);
}

void FaintWindowContext::BeginModalDialog(){
  m_modalDialog += 1;
}

void FaintWindowContext::BeginTextEntry(){
  m_faintWindow.BeginTextEntry();
}

void FaintWindowContext::Close(Canvas& canvas, bool force){
  m_faintWindow.CloseDocument(canvas, force);
}

void FaintWindowContext::DialogOpenFile(){
  FileList paths = show_open_file_dialog(m_faintWindow.GetRawFrame(),
    *this,
    Title("Open Image(s)"),
    get_canvas_dir(GetActiveCanvas()),
    combined_file_dialog_filter(utf8_string("Image files"),
      loading_file_formats(m_faintWindow.GetFileFormats())));

  if (!paths.empty()){
    Load(paths);
  }
}

void FaintWindowContext::DialogSaveAs(Canvas& canvas, bool backup){
  m_faintWindow.ShowSaveAsDialog(canvas, backup);
}

void FaintWindowContext::EndModalDialog(){
  m_modalDialog -= 1;
  assert(m_modalDialog >= 0);
}

void FaintWindowContext::EndTextEntry(){
  m_faintWindow.EndTextEntry();
}

bool FaintWindowContext::Exists(const CanvasId& id){
  return m_faintWindow.Exists(id);
}

bool FaintWindowContext::FaintWindowFocused() const{
  const bool otherFocused =
    ModalDialogShown() ||
    m_helpFrame.HasFocus() ||
    m_interpreterFrame.HasFocus();

  return !otherFocused;
}

BoolSetting::ValueType FaintWindowContext::Get(const BoolSetting& s){
  return m_faintWindow.GetShownSettings().Get(s);
}

StringSetting::ValueType FaintWindowContext::Get(const StringSetting& s){
  return m_faintWindow.GetShownSettings().Get(s);
}

IntSetting::ValueType FaintWindowContext::Get(const IntSetting& s){
  return m_faintWindow.GetShownSettings().Get(s);
}

PaintSetting::ValueType FaintWindowContext::Get(const PaintSetting& s){
  return m_faintWindow.GetShownSettings().Get(s);
}

FloatSetting::ValueType FaintWindowContext::Get(const FloatSetting& s){
  return m_faintWindow.GetShownSettings().Get(s);
}

Interaction& FaintWindowContext::GetInteraction(){
  return m_interaction;
}

ExtraOverlay& FaintWindowContext::GetExtraOverlay(){
  return m_extraOverlay;
}

Canvas& FaintWindowContext::GetActiveCanvas(){
  return m_faintWindow.GetActiveCanvas();
}

Tool* FaintWindowContext::GetActiveTool(){
  return m_faintWindow.GetActiveTool();
}

Canvas& FaintWindowContext::GetCanvas(const Index& i){
  return m_faintWindow.GetCanvas(i);
}

Canvas& FaintWindowContext::GetCanvas(const CanvasId& id){
  return m_faintWindow.GetCanvas(id);
}

Index FaintWindowContext::GetCanvasCount() const{
  return m_faintWindow.GetCanvasCount();
}

Grid FaintWindowContext::GetDefaultGrid() const{
  return m_defaultGrid;
}

ImageInfo FaintWindowContext::GetDefaultImageInfo(){
  return ImageInfo(IntSize(640,480), Color(255,255,255), create_bitmap(true));
}

FaintDialogContext& FaintWindowContext::GetDialogContext(){
  return m_dialogContext;
}

std::vector<Format*> FaintWindowContext::GetFormats(){
  return m_faintWindow.GetFileFormats();
}

ResizeDialogOptions FaintWindowContext::GetDefaultResizeDialogOptions() const{
  return m_defaultResizeSettings;
}

Layer FaintWindowContext::GetLayerType() const{
  return m_faintWindow.GetLayerType();
}

IntPoint FaintWindowContext::GetMousePos(){
  return to_faint(wxGetMousePosition());
}

StatusInterface& FaintWindowContext::GetStatusInfo(){ // Non virtual
  return m_statusbar;
}

ToolId FaintWindowContext::GetToolId() const{
  return m_faintWindow.GetToolId();
}

Settings FaintWindowContext::GetToolSettings() const{
  return m_faintWindow.GetShownSettings();
}

const TransparencyStyle& FaintWindowContext::GetTransparencyStyle() const{
  return m_transparencyStyle;
}

bool FaintWindowContext::IsFullScreen() const{
  return m_faintWindow.IsFullScreen();
}

Canvas* FaintWindowContext::Load(const FilePath& filePath, const change_tab& changeTab){
  return m_faintWindow.Open(filePath, changeTab);
}

void FaintWindowContext::Load(const FileList& filePaths){
  return m_faintWindow.Open(filePaths);
}

Canvas* FaintWindowContext::LoadAsFrames(const FileList& paths,
  const change_tab& changeTab)
{
  std::vector<ImageProps> props;
  const std::vector<Format*> formats =
    loading_file_formats(m_faintWindow.GetFileFormats());
  for (const FilePath& filePath : paths){
    FileExtension extension(filePath.Extension());
    bool loaded = false;
    for (Format* format : formats){
      if (format->Match(extension)){
        props.emplace_back();
        format->Load(filePath, props.back());
        if (!props.back().IsOk()){
          // Fixme: Commented for some reason
          // show_load_failed_error(m_faintWindow, filePath, props.back().GetError());
          return nullptr;
        }
        loaded = true;
        break;
      }
    }
    if (!loaded){
      // show_load_failed_error(m_faintWindow, filePath, "One path could not be loaded.");
      return nullptr;
    }
  }
  auto canvas = m_tabControl->NewDocument(std::move(props),
    changeTab, initially_dirty(true));
  Canvas* canvasInterface = &(canvas->GetInterface());
  return canvasInterface;
}

void FaintWindowContext::Maximize(){
  m_faintWindow.Maximize(!m_faintWindow.IsMaximized());
}

void FaintWindowContext::MaximizePythonConsole(){
  m_interpreterFrame.Maximize(!m_interpreterFrame.IsMaximized());
}

bool FaintWindowContext::ModalDialogShown() const{
  return m_modalDialog > 0;
}

Canvas& FaintWindowContext::NewDocument(const ImageInfo& info){
  return m_faintWindow.NewDocument(info);
}

Canvas& FaintWindowContext::NewDocument(ImageProps&& props){
  CanvasPanel* canvas = m_tabControl->NewDocument(std::move(props),
    change_tab(true), initially_dirty(true));
  return canvas->GetInterface();
}

void FaintWindowContext::NextTab(){
  m_faintWindow.NextTab();
}

void FaintWindowContext::PreviousTab(){
  m_faintWindow.PreviousTab();
}

void FaintWindowContext::QueueLoad(const FileList& filenames){
  m_faintWindow.QueueLoad(filenames);
}

void FaintWindowContext::Quit(bool force){
  m_faintWindow.Close(force);
}

void FaintWindowContext::RaiseWindow(){
  m_faintWindow.Raise();
}

bool FaintWindowContext::Save(Canvas& canvas){
  return m_faintWindow.Save(canvas);
}

void FaintWindowContext::SelectTool(ToolId id){
  m_faintWindow.SelectTool(id);
}

void FaintWindowContext::Set(const BoolSetting& s, BoolSetting::ValueType v){
  change_setting(m_faintWindow, s, v, from_control(false), m_dialogContext);
}

void FaintWindowContext::Set(const StringSetting& s, const StringSetting::ValueType& v){
  change_setting(m_faintWindow, s, v, from_control(false), m_dialogContext);
}

void FaintWindowContext::Set(const IntSetting& s, IntSetting::ValueType v){
  change_setting(m_faintWindow, s, v, from_control(false), m_dialogContext);
}

void FaintWindowContext::Set(const PaintSetting& s, PaintSetting::ValueType v){
  change_setting(m_faintWindow, s, v, from_control(false), m_dialogContext);
}

void FaintWindowContext::Set(const FloatSetting& s, FloatSetting::ValueType v){
  change_setting(m_faintWindow, s, v, from_control(false), m_dialogContext);
}

void FaintWindowContext::SetActiveCanvas(const CanvasId& id){
  return m_faintWindow.SetActiveCanvas(id);
}

void FaintWindowContext::SetDefaultGrid(const Grid& grid){
  m_defaultGrid = grid;
}

void FaintWindowContext::SetDefaultResizeDialogOptions(const ResizeDialogOptions& opts){
  m_defaultResizeSettings = opts;
}

void FaintWindowContext::SetPythonConsoleBackground(const ColRGB& c){
  m_interpreterFrame.SetBackgroundColor(c);
}

void FaintWindowContext::SetPythonConsoleTextColor(const ColRGB& c){
  m_interpreterFrame.SetTextColor(c);
}

void FaintWindowContext::SetPalette(const PaintMap& paintMap){
  m_faintWindow.SetPalette(paintMap);
}

void FaintWindowContext::SetTransparencyStyle(const TransparencyStyle& style){
  m_transparencyStyle = style;
  m_faintWindow.Refresh();
}

void FaintWindowContext::SetLayer(Layer layer){
  m_faintWindow.SelectLayer(layer);
}

void FaintWindowContext::ModalFull(const dialog_func& show_dialog){
  Canvas& canvas = m_faintWindow.GetActiveCanvas();
  DialogFeedbackImpl feedback(canvas);
  BeginModalDialog();
  Optional<Command*> maybeCmd = show_dialog(m_faintWindow.GetRawFrame(),
    feedback, canvas);
  EndModalDialog();
  if (maybeCmd.NotSet()){
    canvas.Refresh();
    return;
  }
  canvas.RunCommand(maybeCmd.Get());
  canvas.Refresh();
}

void FaintWindowContext::Modal(const bmp_dialog_func& show_dialog){
  Canvas& canvas(GetActiveCanvas());
  DialogFeedbackImpl feedback(canvas);

  auto maybeCmd = show_dialog(m_faintWindow.GetRawFrame(),
    m_dialogContext,
    feedback);

  maybeCmd.Visit(
    [&](BitmapCommand* cmd){
      canvas.RunCommand(context_targetted(cmd, canvas));
      canvas.Refresh();
    },
    [&](){
      canvas.Refresh();
    });
}

void FaintWindowContext::ToggleHelpFrame(){
  toggle_top_level_window(m_helpFrame);
}

void FaintWindowContext::TogglePythonConsole(){
  toggle_top_level_window(m_interpreterFrame);
}

void FaintWindowContext::ShowColorPanel(bool show){
  m_faintWindow.ShowColorPanel(show);
}

void FaintWindowContext::ShowPythonConsole(){
  m_interpreterFrame.Show();
  m_interpreterFrame.Raise();
}

void FaintWindowContext::ShowStatusbar(bool show){
  m_faintWindow.ShowStatusbar(show);
}

void FaintWindowContext::ShowToolPanel(bool show){
  m_faintWindow.ShowToolPanel(show);
}

int FaintWindowContext::TabletGetCursor(){
  return m_tabletCursor;
}

// Note: Not an override, used directly by FaintWindow
void FaintWindowContext::TabletSetCursor(int tabletCursor){
  m_tabletCursor = tabletCursor;
}

void FaintWindowContext::ToggleFullScreen(bool fullScreen){
  m_faintWindow.FullScreen(fullScreen);
}

void FaintWindowContext::ToggleMaximize(){
  m_faintWindow.Maximize(!m_faintWindow.IsMaximized());
}

void FaintWindowContext::UpdateShownSettings(){
  m_faintWindow.UpdateShownSettings();
}

void FaintWindowContext::UpdateToolSettings(const Settings& s){
  m_faintWindow.UpdateToolSettings(s);
}

void FaintWindowContext::SetTabCtrl(TabCtrl* tabControl){ // Non virtual
  // Fixme: Remove this method.
  // Added to make it possible to remove GetTabCtrl from FaintWindow,
  // (the initialization order of AppContext and panels
  // are a mess).
  m_tabControl.reset(tabControl);
}

} // namespace
