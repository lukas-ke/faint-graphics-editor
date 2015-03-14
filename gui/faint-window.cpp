// -*- coding: us-ascii-unix -*-
// Copyright 2009 Lukas Kemmer
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

#include <algorithm>
#include "wx/frame.h"
#include "wx/filename.h"
#include "wx/filedlg.h"
#include "wx/statusbr.h"
#include "wx/sizer.h"
#include "app/active-canvas.hh"
#include "gui/canvas-panel.hh"
#include "gui/color-panel.hh"
#include "gui/events.hh"
#include "gui/faint-window.hh"
#include "gui/faint-window-app-context.hh"
#include "gui/faint-window-python-context.hh"
#include "gui/freezer.hh"
#include "gui/help-frame.hh"
#include "gui/menu-bar.hh"
#include "gui/tab-ctrl.hh"
#include "gui/tool-panel.hh"
#include "gui/setting-events.hh"
#include "tablet/tablet-event.hh"
#include "text/formatting.hh"
#include "util/cleaner.hh"
#include "util/color-choice.hh"
#include "util/convenience.hh"
#include "util/default-settings.hh"
#include "util/frame-props.hh"
#include "util/image-props.hh"
#include "util/index-iter.hh"
#include "util/mouse.hh"
#include "util-wx/bind-event.hh"
#include "util-wx/clipboard.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/file-format-util.hh"
#include "util-wx/gui-util.hh"

#ifdef __WXMSW__
#include "wx/msw/private.h"
#include "tablet/msw/tablet-error-message.hh"
#include "tablet/msw/tablet-init.hh"
#endif

namespace faint{

struct FaintPanels{
  std::unique_ptr<Menubar> menubar;
  std::unique_ptr<TabCtrl> tabControl;
  std::unique_ptr<ToolPanel> tool;
  std::unique_ptr<ColorPanel> color;
};

struct FaintFloatingWindows{
  FaintFloatingWindows(HelpFrame& help, InterpreterFrame& interpreter)
    : help(help),
      interpreter(interpreter)
  {}

  void CloseAll(){
    help.Close();
    interpreter.Close();
  }

  HelpFrame& help;
  InterpreterFrame& interpreter;
};

static CanvasPanel& get_active_canvas_panel(const FaintPanels& p){
  auto active = p.tabControl->GetActiveCanvas();
  assert(active != nullptr);
  return *active;
}

static Canvas& get_active_canvas(const FaintPanels& p){
  return get_active_canvas_panel(p).GetInterface();
}

static Index get_canvas_count(const FaintPanels& p){
  return p.tabControl->GetCanvasCount();
}

struct WindowSettings {
  WindowSettings(){
    toolbar_visible = palette_visible = statusbar_visible = true;
  }
  bool toolbar_visible;
  bool palette_visible;
  bool statusbar_visible;
};

class FaintState{
public:
  FaintState(const Settings& s, bool silentMode)
    : toolSettings(s),
      silentMode(silentMode)
  {}
  Tool* activeTool = nullptr; // Fixme: Who deletes him?
  Layer layer = Layer::RASTER;
  Settings toolSettings;
  std::vector<Format*> formats;
  WindowSettings windowSettings;
  const bool silentMode;
  int textEntryCount = 0;
};

static wxStatusBar& create_faint_statusbar(wxFrame* frame){
  // The status bar is cleaned up by wxWidgets at frame destruction.
  wxStatusBar* statusbar = new wxStatusBar(frame, wxID_ANY);
  frame->SetStatusBar(statusbar);
  statusbar->SetFieldsCount(2);
  return *statusbar;
}

static void change_settings(FaintWindow& window, const Settings& newSettings){
  Tool* tool = window.GetActiveTool();
  bool toolModified = tool->UpdateSettings(newSettings);
  if (toolModified){
    window.GetActiveCanvas().Refresh();
    if (tool->EatsSettings()){
      window.UpdateShownSettings();
      return;
    }
  }
  window.GetToolSettings().Update(newSettings);
  window.UpdateShownSettings();
}

static void update_menu(FaintPanels& p){
  p.menubar->Update(get_active_canvas_panel(p).GetMenuFlags());
}

static void update_canvas_state(const CanvasId& id,
  FaintPanels& panels,
  FaintState& state)
{
  if (get_active_canvas_panel(panels).GetCanvasId() == id){
    update_menu(panels);
  }
  panels.tool->ShowSettings(state.activeTool->GetSettings());
  panels.color->UpdateFrames();
}

static void update_shown_settings(FaintState& state, FaintPanels& panels){
  const Settings& activeSettings(state.activeTool->GetSettings());
  panels.tool->ShowSettings(activeSettings);
  panels.color->UpdateSelectedColors(get_color_choice(activeSettings,
    state.toolSettings, state.activeTool->EatsSettings()));
}

static void update_tool_settings(const Settings& s, FaintState& state,
  FaintPanels& panels)
{
  state.toolSettings.Update(s);
  state.activeTool->UpdateSettings(state.toolSettings);
  update_shown_settings(state, panels);
}

static void update_zoom(FaintPanels& panels){
  ZoomLevel zoom(get_active_canvas_panel(panels).GetZoomLevel());
  panels.color->UpdateZoom(zoom);
  panels.menubar->UpdateZoom(zoom);
}

static void initialize_panels(wxFrame& frame, FaintWindowContext& app,
  FaintPanels& panels,
  FaintState& state,
  ArtContainer& art,
  const PaintMap& palette)
{
  panels.menubar = std::make_unique<Menubar>(frame, app, art);

  // Top half, the tool panel and the drawing areas.
  auto* row1 = new wxBoxSizer(wxHORIZONTAL);
  panels.tool = std::make_unique<ToolPanel>(&frame,
    app.GetStatusInfo(),
    art,
    app.GetDialogContext());
  row1->Add(panels.tool->AsWindow(), 0, wxEXPAND);

  panels.tabControl = std::make_unique<TabCtrl>(&frame,
    art,
    app,
    app.GetStatusInfo());
  row1->Add(panels.tabControl->AsWindow(), 1, wxEXPAND);

  // Bottom half, the selected color, palette and zoom controls.
  panels.color = std::make_unique<ColorPanel>(&frame,
    state.toolSettings,
    palette,
    app,
    app.GetStatusInfo(),
    art);

  auto rows = new wxBoxSizer(wxVERTICAL);
  rows->Add(row1, 1, wxEXPAND);
  rows->Add(panels.color->AsWindow(), 0, wxEXPAND);
  frame.SetSizer(rows);
  frame.Layout();

  app.SetTabCtrl(panels.tabControl.get()); // Fixme: Remove

  // Fixme: Wrong place for such stuff, has nothing todo with main
  // frame. Consider App.
  state.formats = built_in_file_formats();
  frame.SetMinSize(wxSize(640, 480));
}

class FaintFrame : public wxFrame {
public:
  FaintFrame()
    : wxFrame(nullptr, wxID_ANY, "Faint", wxPoint(50,50), wxSize(800,700))
  {}

  #ifdef __WXMSW__
  WXLRESULT MSWWindowProc(WXUINT msg, WXWPARAM wParam, WXLPARAM lParam) override{
    // MS-Windows-specific pen-tablet handling
    if (tablet::is_wt_packet(msg)){
      tablet::WTP packet = tablet::get_wt_packet(wParam, lParam);
      ScreenToClient(&packet.x, &packet.y);
      TabletEvent event(packet.x, packet.y, packet.pressure, packet.cursor);
      AddPendingEvent(event);
    }
    if (msg == WM_ACTIVATE){
      tablet::msg_activate(wParam, lParam);
    }
    return wxFrame::MSWWindowProc(msg, wParam, lParam);
  }
  #endif
};

// Fixme: return the error instead of taking parent, showing error message
static bool save(wxWindow* parent,
  Format* format,
  Canvas& canvas,
  const FilePath& filePath,
  bool backup=false)
{
  SaveResult result = format->Save(filePath, canvas);
  if (result.Failed()){
    show_error(parent, Title("Failed Saving"),
      to_wx(result.ErrorDescription()));
    return false;
  }
  if (!backup){
    canvas.NotifySaved(filePath);
  }
  return true;
}

class FaintWindowImpl{
public:
  FaintWindowImpl(FaintFrame* f,
    std::unique_ptr<FaintPanels>&& in_panels,
    std::unique_ptr<FaintState>&& in_state,
    FaintWindowContext& app,
    PythonContext& python,
    HelpFrame& help,
    InterpreterFrame& interpreter)
    : appContext(app),
      panels(std::move(in_panels)),
      pythonContext(python),
      state(std::move(in_state)),
      windows(help, interpreter)
  {
    frame.reset(f);
  }

  FaintDialogContext& GetDialogContext(){
    return appContext.GetDialogContext();
  }

  FaintWindowContext& appContext;
  dumb_ptr<FaintFrame> frame;
  std::unique_ptr<FaintPanels> panels;
  PythonContext& pythonContext;
  std::unique_ptr<FaintState> state;
  FaintFloatingWindows windows;
  std::vector<std::unique_ptr<Cleaner>> cleanup;

};

static void select_tool(ToolId id, FaintState& state, FaintPanels& panels,
  AppContext& app)
{
  if (state.activeTool != nullptr){
    auto& canvas = get_active_canvas_panel(panels);
    canvas.Preempt(PreemptOption::ALLOW_COMMAND);
    delete state.activeTool;
    state.activeTool = nullptr;
  }

  state.activeTool = new_tool(id,
    state.layer,
    state.toolSettings,
    ActiveCanvas(app));
}

FaintWindow::FaintWindow(ArtContainer& art,
  const PaintMap& palette,
  HelpFrame* helpFrame,
  InterpreterFrame* interpreterFrame,
  bool silentMode)
{
  // Deleted by wxWidgets
  auto* frame = new FaintFrame();

  std::unique_ptr<FaintPanels> panels(new FaintPanels);
  std::unique_ptr<FaintState> state(new FaintState(default_tool_settings(),
      silentMode));

  // Fixme: Who deletes?
  FaintWindowContext* appContext = new FaintWindowContext(*this,
    create_faint_statusbar(frame), *helpFrame, *interpreterFrame);

  FaintWindowPythonContext* pythonContext = new
    FaintWindowPythonContext(*this, *interpreterFrame);

  initialize_panels(*frame, *appContext, *panels, *state, art, palette);

  m_impl = std::make_unique<FaintWindowImpl>(frame,
    std::move(panels), std::move(state),
    *appContext, *pythonContext,
    *helpFrame, *interpreterFrame);

  bind_fwd(frame, EVT_FAINT_ADD_TO_PALETTE,
    [&](const PaintEvent& event){
      AddToPalette(event.GetPaint());
  });

  bind_fwd(frame, EVT_FAINT_OPEN_FILES,
    [&](const OpenFilesEvent& event){
      Open(event.GetFileNames());
    });

  bind_fwd(frame, EVT_FAINT_TOOL_CHANGE,
    [&](const ToolChangeEvent& event){
      select_tool(event.GetTool(), *m_impl->state,
        *m_impl->panels, m_impl->appContext);
      UpdateShownSettings();
    });

  bind_fwd(frame, EVT_FAINT_COPY_COLOR_HEX,
    [this](const ColorEvent& event){
      Clipboard clipboard;
      if (!clipboard.Good()){
        show_copy_color_error(m_impl->frame.get());
        return;
      }
      clipboard.SetText(str_hex(event.GetColor()));
    });

  bind_fwd(frame, EVT_FAINT_COPY_COLOR_RGB,
    [this](const ColorEvent& event){
      Clipboard clipboard;
      if (!clipboard.Good()){
        show_copy_color_error(m_impl->frame.get());
        return;
      }
      clipboard.SetText(str_smart_rgba(event.GetColor()));
    });

  bind(frame, EVT_SWAP_COLORS,
    [this](){
      auto& app(m_impl->appContext);
      auto oldFg = app.Get(ts_Fg);
      auto oldBg = app.Get(ts_Bg);
      app.Set(ts_Fg, oldBg);
      app.Set(ts_Bg, oldFg);
    });

  bind_fwd(frame, EVT_FAINT_GRID_CHANGE,
    [this](CanvasChangeEvent& e){
      auto& panels(*m_impl->panels);
      if (e.GetCanvasId() == get_active_canvas(panels).GetId()){
        panels.color->UpdateGrid();
      }
    });

  bind(frame, EVT_SET_FOCUS_ENTRY_CONTROL,
    [this](){
      if (m_impl->state->textEntryCount == 0){
        // Entry controls are numeric, not all shortcuts need to be
        // disabled
        const bool numeric = true;
        m_impl->panels->menubar->BeginTextEntry(numeric);
      }
      m_impl->state->textEntryCount++;
    });

  bind(frame, EVT_KILL_FOCUS_ENTRY_CONTROL,
    [this](){
      EndTextEntry();
    });

  bind_fwd(frame, EVT_FAINT_ACTIVE_CANVAS_CHANGE,
    [this](CanvasChangeEvent& e){
      auto& panels(*m_impl->panels);
      auto& state(*m_impl->state);
      update_zoom(panels);
      panels.color->UpdateGrid();
      update_canvas_state(e.GetCanvasId(), panels, state);
      state.activeTool->SelectionChange();
      update_shown_settings(state, panels);
      m_impl->GetDialogContext().Reinitialize();

    });

  bind_fwd(frame, EVT_FAINT_CANVAS_CHANGE,
    [this](CanvasChangeEvent& e){
      auto& panels(*m_impl->panels);
      auto& state(*m_impl->state);
      update_canvas_state(e.GetCanvasId(), panels, state);
      update_shown_settings(state, panels);
      m_impl->GetDialogContext().Reinitialize();
    });

  bind_fwd(frame, EVT_FAINT_ZOOM_CHANGE,
    [this](CanvasChangeEvent& e){
      auto& panels(*m_impl->panels);
      if (e.GetCanvasId() == get_active_canvas(panels).GetId()){
        update_zoom(panels);
      }
    });

  bind_fwd(frame, EVT_FAINT_LAYER_CHANGE,
    [&](const LayerChangeEvent event){
      auto& state(*m_impl->state);
      auto& panels(*m_impl->panels);
      state.layer = event.GetLayer();
      CanvasPanel& activeCanvas = get_active_canvas_panel(panels);
      activeCanvas.MousePosRefresh();
      activeCanvas.Refresh();
      update_canvas_state(activeCanvas.GetCanvasId(), panels, state);
    });

  bind_fwd(frame, wxEVT_CLOSE_WINDOW,
    [&](wxCloseEvent& event){
      // Exiting Faint

      // Ask for unsaved changes (if possible), potentially cancelling
      // the shut-down.
      auto& panels(*m_impl->panels);

      if (panels.tabControl->UnsavedDocuments() && event.CanVeto()){
        bool quit = ask_exit_unsaved_changes(m_impl->frame.get());
        if (!quit){
          event.Veto();
          return;
        }
      }

      panels.menubar->StoreRecentFiles();
      Clipboard::Flush();

      // Close floating non-command windows (e.g. help, interpreter)
      m_impl->windows.CloseAll();

      // Close any dialogs
      m_impl->GetDialogContext().Close();

      event.Skip();
    });

  bind_fwd(frame, EVT_TABLET,
    [=](const TabletEvent& event){
      appContext->TabletSetCursor(event.GetCursor());
    });

  auto* toolPanel = m_impl->panels->tool->AsWindow();

  bind_fwd(toolPanel, EVT_FAINT_INT_SETTING_CHANGE,
    [this](const SettingEvent<IntSetting>& e){
      change_setting(*this, e.GetSetting(), e.GetValue(), from_control(true),
        m_impl->appContext.GetDialogContext());
    });

  bind_fwd(toolPanel, EVT_FAINT_FLOAT_SETTING_CHANGE,
    [this](const SettingEvent<FloatSetting>& e){
      change_setting(*this, e.GetSetting(), e.GetValue(), from_control(true),
        m_impl->appContext.GetDialogContext());
    });

  bind_fwd(toolPanel, EVT_FAINT_BOOL_SETTING_CHANGE,
    [this](const SettingEvent<BoolSetting>& e){
      change_setting(*this, e.GetSetting(), e.GetValue(), from_control(true),
        m_impl->appContext.GetDialogContext());
    });

  bind_fwd(toolPanel, EVT_FAINT_STRING_SETTING_CHANGE,
    [this](const SettingEvent<StringSetting>& e){
      change_setting(*this, e.GetSetting(), e.GetValue(), from_control(true),
        m_impl->appContext.GetDialogContext());
    });

  bind_fwd(toolPanel,EVT_FAINT_SETTINGS_CHANGE,
    [this](const SettingsEvent& e){
      // Note that m_fromCtrl is not used when changing multiple
      // settings. This is because font dialog can change the font
      // size, which is also shown in another control which must be
      // updated, so there's no point in trying to not refresh controls.
      change_settings(*this, e.GetSettings());
    });

  bind_fwd(m_impl->panels->color->AsWindow(), EVT_FAINT_COLOR_SETTING_CHANGE,
    [this](const SettingEvent<ColorSetting>& e){
      change_setting(*this, e.GetSetting(), e.GetValue(), from_control(false),
        m_impl->appContext.GetDialogContext());
    });

  m_impl->panels->tool->SelectTool(ToolId::LINE);
  restore_persisted_state(m_impl->frame.get(), storage_name("FaintWindow"));
}

FaintWindow::~FaintWindow(){
  // This destructor must be defined or unique_ptr to forward-declared
  // FaintWindowImpl won't compile.
}

void FaintWindow::EnableToolbar(bool e){
  m_impl->panels->tool->Enable(e);
}

void FaintWindow::AddFormat(Format* f){
  m_impl->state->formats.push_back(f);
}

void FaintWindow::AddToPalette(const Paint& paint){
  m_impl->panels->color->AddToPalette(paint);
}

void FaintWindow::BeginTextEntry(){
  m_impl->state->textEntryCount++;
  m_impl->panels->menubar->BeginTextEntry();

  // To enable e.g. "Select none" for the EditText task, bit of a hack.
  update_menu(*m_impl->panels);
}

void FaintWindow::Close(bool force){
  m_impl->frame->Close(force);
}

void FaintWindow::CloseDocument(Canvas& canvas){
  FaintPanels& panels(*m_impl->panels);
  auto& tabControl = *(panels.tabControl);
  for (auto i : up_to(tabControl.GetCanvasCount())){
    Canvas& other = tabControl.GetCanvas(i)->GetInterface();
    if (&other == &canvas){
      // Freeze the color panel to avoid the FrameControl refreshing
      // during page close on GTK. (See issue 122).
      auto freezer = freeze(panels.color);
      tabControl.Close(i);
      return;
    }
  }
}

void FaintWindow::EndTextEntry(){
  auto& textEntryCount(m_impl->state->textEntryCount);
  assert(textEntryCount > 0);
  textEntryCount--;
  if (textEntryCount == 0){
    m_impl->panels->menubar->EndTextEntry();
  }
}

bool FaintWindow::Exists(const CanvasId& id) const{
  return m_impl->panels->tabControl->Has(id);
}

bool FaintWindow::Focused() const{
  // Fixme: Can probably be done here, without appContext.
  return m_impl->appContext.FaintWindowFocused();
}

void FaintWindow::FullScreen(bool enable){
  auto& panels(*m_impl->panels);
  auto* frame(m_impl->frame.get());
  const auto& windowSettings(m_impl->state->windowSettings);
  if (enable){
    auto freezer = freeze(frame);
    panels.tool->Hide();
    panels.color->Hide();
    panels.tabControl->HideTabs();
  }
  else {
    auto freezer = freeze(frame);
    panels.color->Show(windowSettings.palette_visible);
    panels.tool->Show(windowSettings.toolbar_visible);
    panels.tabControl->ShowTabs();
  }
  frame->ShowFullScreen(enable);
}

Canvas& FaintWindow::GetActiveCanvas(){
  return get_active_canvas(*m_impl->panels);
}

Tool* FaintWindow::GetActiveTool(){
  return m_impl->state->activeTool;
}

AppContext& FaintWindow::GetAppContext(){
  return m_impl->appContext;
}

Canvas& FaintWindow::GetCanvas(const Index& i){
  return m_impl->panels->tabControl->GetCanvas(i)->GetInterface();
}

Index FaintWindow::GetCanvasCount() const{
  return get_canvas_count(*m_impl->panels);
}

std::vector<Format*>& FaintWindow::GetFileFormats(){
  return m_impl->state->formats;
}

wxFrame& FaintWindow::GetRawFrame(){
  assert(m_impl != nullptr); // Called during FaintWindow-constructor?
  return *(m_impl->frame);
}

Layer FaintWindow::GetLayerType() const{
  return m_impl->state->layer;
}

PythonContext& FaintWindow::GetPythonContext(){
  return m_impl->pythonContext;
}

Settings FaintWindow::GetShownSettings() const{
  const auto& state(*(m_impl->state));
  Settings settings(state.toolSettings);
  settings.Update(state.activeTool->GetSettings());
  return settings;
}

EntryMode FaintWindow::GetTextEntryMode() const{
  return m_impl->panels->menubar->GetTextEntryMode();
}

ToolId FaintWindow::GetToolId() const{
  const auto& state(*m_impl->state);
  if (state.activeTool == nullptr){
    return ToolId::OTHER;
  }
  return state.activeTool->GetId();
}

Settings& FaintWindow::GetToolSettings(){
  return m_impl->state->toolSettings;
}

void FaintWindow::Initialize(){
  // Pretty much all of Faint expects there to be an active canvas, so
  // create one early. It would be nicer if no blank "untitled"-image
  // was created when starting with files from the command line, but
  // this caused crashes if the loading failed (even with attempt to
  // open a blank document on failure)
  // ...Perhaps the loading should be detached from the FaintWindow and
  // everything, but that's for some other time.
  NewDocument(m_impl->appContext.GetDefaultImageInfo());

  #ifdef __WXMSW__
  // Fixme: Add common tablet init which is no-op on Linux (for now)
  if (!m_impl->state->silentMode){
    tablet::InitResult tabletResult = tablet::initialize(wxGetInstance(),
      m_impl->frame->GetHWND());

    if (tabletResult == tablet::InitResult::OK){
      tablet::activate(true);
      m_impl->cleanup.emplace_back(
        create_cleaner([](){
          tablet::uninitialize();
          }));
    }
    else{
      tablet::show_tablet_error_message(m_impl->frame.get(), tabletResult);
    }
  }
  #endif
}

bool FaintWindow::IsFullScreen() const{
  return m_impl->frame->IsFullScreen();
}

bool FaintWindow::IsMaximized() const{
  return m_impl->frame->IsMaximized();
}

void FaintWindow::Maximize(bool maximize){
  m_impl->frame->Maximize(maximize);
}

void FaintWindow::ModifierKeyChange(){
  if (!Focused()){
    return;
  }

  auto& active = get_active_canvas_panel(*m_impl->panels);
  if (active.HasFocus()){
    // The key-handler of the CanvasPanel will get this
    // keypress.
    return;
  }

  IntPoint mousePos = mouse::screen_position();
  if (active.GetScreenRect().Contains(to_wx(mousePos))){
    active.MousePosRefresh();
  }
}

Canvas& FaintWindow::NewDocument(const ImageInfo& info){
  auto& tabControl = (*m_impl->panels->tabControl);
  CanvasPanel* canvas = tabControl.NewDocument(ImageProps(info),
    change_tab(true), initially_dirty(false));
  assert(canvas != nullptr);
  return canvas->GetInterface();
}

void FaintWindow::NextTab() {
  m_impl->panels->tabControl->SelectNext();
}

void FaintWindow::Open(const FileList& paths){
  auto& panels(*m_impl->panels);

  if (panels.tabControl->GetCanvasCount() > 0){
    // Refresh the entire frame to erase any dialog or menu droppings before
    // starting potentially long-running file loading.
    Refresh();
    m_impl->frame->Update();
  }

  FileList notFound; // Fixme: Ugly fix for issue 114 (though bundling file names is nicer than individual error messages)
  try {
    // Freeze the panel to remove some refresh glitches in the
    // tool-settings on MSW during loading.
    auto freezer = freeze(panels.tool->AsWindow());
    bool first = true;
    for (const FilePath& filePath : paths){
      if (exists(filePath)){
        Open(filePath, change_tab(then_false(first)));
      }
      else {
        notFound.push_back(filePath); // Fixme
      }
    }
  }
  catch (const std::bad_alloc&){
    show_error(m_impl->frame.get(), Title("Insufficient memory to load "
      "all images."), "Out of memory");
  }

  if (get_canvas_count(panels) == 0){
    NewDocument(m_impl->appContext.GetDefaultImageInfo());
  }

  if (notFound.size() == 1){
    show_file_not_found_error(m_impl->frame.get(), notFound.back());
  }
  else if (notFound.size() > 1){
    utf8_string error = "Files not found: \n";
    for (const FilePath& path : notFound){
      error += path.Str() + "\n";
    }
    show_error(m_impl->frame.get(), Title("Files not found"), to_wx(error));
  }
}

Canvas* FaintWindow::Open(const FilePath& filePath,
  const change_tab& changeTab)
{
  auto& panels(*m_impl->panels);
  auto& state(*m_impl->state);
  FileExtension extension(filePath.Extension()); // Fixme

  for (Format* format : loading_file_formats(m_impl->state->formats)){
    if (format->Match(extension)){
      ImageProps props;
      format->Load(filePath, props);
      if (!props.IsOk()){
        if (!m_impl->state->silentMode){
          show_load_failed_error(m_impl->frame.get(),
            filePath, props.GetError());
        }
        return nullptr;
      }
      CanvasPanel* newCanvas = panels.tabControl->NewDocument(std::move(props),
        changeTab, initially_dirty(false));
      // Fixme: Either check for null or return reference
      newCanvas->NotifySaved(filePath);
      panels.menubar->AddRecentFile(filePath);
      if (props.GetNumWarnings() != 0){
        if (!state.silentMode){
          show_load_warnings(m_impl->frame.get(), props);
        }
      }
      return &(newCanvas->GetInterface());
    }
  }
  if (!state.silentMode){
    show_file_not_supported_error(m_impl->frame.get(), filePath);
  }
  return nullptr;
}

void FaintWindow::PreviousTab(){
  m_impl->panels->tabControl->SelectPrevious();
}

void FaintWindow::QueueLoad(const FileList& filenames){
  m_impl->frame->GetEventHandler()->QueueEvent(new OpenFilesEvent(filenames));
}

void FaintWindow::Raise(){
  m_impl->frame->Raise();
}

void FaintWindow::Refresh(){
  m_impl->frame->Refresh();
}

bool FaintWindow::Save(Canvas& canvas){
  auto& state(*m_impl->state);
  Optional<FilePath> maybeFilePath(canvas.GetFilePath());

  return maybeFilePath.Visit(
    [&](const FilePath& p){
      Format* format = get_save_format(state.formats, p.Extension());
      if (format == nullptr){
        return ShowSaveAsDialog(canvas);
      }
      bool savedOk = save(m_impl->frame.get(), format, canvas, p);
      if (!savedOk){
        return false;
      }
      if (format->CanLoad() || get_load_format(state.formats,
          p.Extension()) != nullptr){
        m_impl->panels->menubar->AddRecentFile(p);
      }
      return true;
    },
    [&](){
      return ShowSaveAsDialog(canvas);
    });
}

bool FaintWindow::ShowSaveAsDialog(Canvas& canvas, bool backup){
  auto& state(*m_impl->state);
  Optional<FilePath> oldFilePath(canvas.GetFilePath());
  wxFileName initialPath(oldFilePath.IsSet() ?
    wxFileName(to_wx(oldFilePath.Get().Str())) :
    wxFileName());
  int defaultFormatIndex = oldFilePath.NotSet() ? 0 :
    get_file_format_index(state.formats, FileExtension(initialPath.GetExt()));

  wxFileDialog dlg(m_impl->frame.get(),
    "Save as",
    initialPath.GetPath(),
    initialPath.GetName(),
    to_wx(file_dialog_filter(saving_file_formats(state.formats))),
    wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

  if (defaultFormatIndex != -1){
    dlg.SetFilterIndex(defaultFormatIndex);
  }

  int result = m_impl->appContext.GetDialogContext().ShowModal(dlg);
  if (result != wxID_OK){
    return false;
  }

  FilePath path(FilePath::FromAbsoluteWx(dlg.GetPath()));
  Format* format = get_save_format(state.formats,
    path.Extension(),
    dlg.GetFilterIndex());
  if (format == nullptr){
    // TODO: Need an error message, though: Can this happen?
    // The dialog will either give the specified extension or
    // the selected extension if none matches.
    return false;
  }

  return save(m_impl->frame.get(), format, canvas, path, backup);
}

void FaintWindow::SelectLayer(Layer layer){
  m_impl->panels->tool->SelectLayer(layer);
}

void FaintWindow::SelectTool(ToolId id){
  m_impl->panels->tool->SelectTool(id);
}

void FaintWindow::SetActiveCanvas(const CanvasId& id){
  m_impl->panels->tabControl->Select(id);
}

void FaintWindow::Show(){
  m_impl->frame->Show();
}

void FaintWindow::SetIcons(const wxIcon& icon16, const wxIcon& icon32){
  m_impl->frame->SetIcons(bundle_icons(icon16, icon32));
}

void FaintWindow::SetPalette(const PaintMap& paintMap){
  m_impl->panels->color->SetPalette(paintMap);
}

void FaintWindow::ShowColorPanel(bool show){
  m_impl->state->windowSettings.palette_visible = show;
  m_impl->panels->color->Show(show);
  m_impl->frame->Layout();
}

void FaintWindow::ShowToolPanel(bool show){
  // Update the setting state and and tool panel visibility. Freezing
  // the FaintFrame removes a refresh-artifact when re-showing the
  // tool-panel on MSW (The wxStaticLine:s from the child panels
  // appeared in the canvas).
  {
    auto freezer = freeze(m_impl->frame);
    m_impl->state->windowSettings.toolbar_visible = show;
    m_impl->panels->tool->Show(show);
  }

  // Update the sizers to the new state of the tool panel. Freezing
  // the color panel removes flicker in the palette. This depends on
  // the tool panel stretching across the frame.
  auto freezer = freeze(m_impl->panels->color);
  m_impl->frame->Layout();
}

void FaintWindow::ShowStatusbar(bool show){
  auto& frame = *(m_impl->frame);
  m_impl->state->windowSettings.statusbar_visible = show;
  frame.GetStatusBar()->Show(show);
  frame.Layout();
}

void FaintWindow::ToggleMaximize(){
  if (!IsFullScreen()){
    Maximize(!IsMaximized());
  }
  else {
    FullScreen(false);
  }
}

void FaintWindow::UpdateShownSettings(){
  update_shown_settings(*m_impl->state, *m_impl->panels);
}

void FaintWindow::UpdateToolSettings(const Settings& s){
  update_tool_settings(s, *m_impl->state, *m_impl->panels);
}

} // namespace
