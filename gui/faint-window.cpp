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
#include <tuple>
#include "wx/frame.h"
#include "wx/filename.h"
#include "wx/filedlg.h"
#include "wx/statusbr.h"
#include "wx/sizer.h"
#include "app/active-canvas.hh"
#include "app/context-grid-dialog.hh"
#include "app/faint-frame-context.hh"
#include "app/faint-window-app-context.hh"
#include "app/faint-tool-actions.hh"
#include "app/faint-window-python-context.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/bitmap-exception.hh"
#include "gui/canvas-panel.hh"
#include "gui/color-panel.hh"
#include "gui/events.hh"
#include "gui/faint-window.hh"
#include "gui/freezer.hh"
#include "gui/grid-dialog.hh"
#include "gui/help-frame.hh"
#include "gui/menu-bar.hh"
#include "gui/paint-dialog.hh"
#include "gui/setting-events.hh"
#include "gui/tab-ctrl.hh"
#include "gui/tool-panel.hh"
#include "tablet/tablet-event.hh"
#include "text/formatting.hh"
#include "text/text-expression-conversions.hh" // Fixme: For unit_px
#include "util/cleaner.hh"
#include "util/color-choice.hh"
#include "util/convenience.hh"
#include "util/default-settings.hh"
#include "util/frame-props.hh"
#include "util/image-props.hh"
#include "util/index-iter.hh"
#include "util/mouse.hh"
#include "util/string-source.hh"
#include "util-wx/bind-event.hh"
#include "util-wx/clipboard.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/file-format-util.hh"
#include "util-wx/fwd-bind.hh"
#include "util-wx/fwd-wx.hh"
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
  FaintState(const Settings& s, bool silentMode, bool usePenTablet)
    : toolSettings(s),
      silentMode(silentMode),
      usePenTablet(!silentMode && usePenTablet)
  {}
  Tool* activeTool = nullptr; // Fixme: Who deletes him?
  Layer layer = Layer::RASTER;
  Settings toolSettings;
  std::vector<Format*> formats;
  WindowSettings windowSettings;
  const bool silentMode;
  int textEntryCount = 0;
  bool usePenTablet;
};

static wxStatusBar& create_faint_statusbar(wxFrame* frame){
  // The status bar is cleaned up by wxWidgets at frame destruction.
  auto statusbar = make_wx<wxStatusBar>(frame, wxID_ANY);
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

// Fixme: Not here, and base units on calibration
class UnitSource : public StringSource{
public:
  std::vector<utf8_string> Get() const override{
    return {unit_px, "mm", "cm", "m"};
  }
};

static void initialize_panels(wxFrame& frame,
  FaintWindowContext& app,
  FaintPanels& panels,
  Art& art,
  const PaintMap& palette)
{
  panels.menubar = std::make_unique<Menubar>(frame, app, art);
  // Top half, the tool panel and the drawing areas.
  auto* row1 = make_wx<wxBoxSizer>(wxHORIZONTAL);

  static UnitSource unitStrings;
  panels.tool = std::make_unique<ToolPanel>(&frame,
    app.GetStatusInfo(),
    art,
    app.GetDialogContext(),
    unitStrings);
  row1->Add(panels.tool->AsWindow(), 0, wxEXPAND);

  panels.tabControl = std::make_unique<TabCtrl>(&frame,
    art,
    app,
    app.GetStatusInfo());
  row1->Add(panels.tabControl->AsWindow(), 1, wxEXPAND);

  auto pickPaint =
    [&app, &frame](const utf8_string& title,
      const Paint& initial,
      const Color& secondary)
    {
      auto getBitmap = [&app](){
        const auto& background = app.GetActiveCanvas().GetBackground();
        return background.Visit(
          [&](const Bitmap& bmp) -> Bitmap{
            return bmp;
          },
          [&](const ColorSpan& colorSpan){
            // Not using the colorSpan size, since it might be huge, and
            // it would be pointless with only one color.
            const IntSize size(1,1);
            return Bitmap(size, colorSpan.color);
          });
      };

      return show_paint_dialog(&frame,
        title,
        initial,
        secondary,
        getBitmap,
        app.GetStatusInfo(),
        app.GetDialogContext());
    };


  auto showGridDialog = [&](){
    context_show_grid_dialog(app.GetActiveCanvas(), app.GetDialogContext());
  };

  auto getBg = [&](){
    return get_color_default(app.Get(ts_Bg), color_white);
  };

  auto gridAccessor = Accessor<Grid>{
    [&](){
      return app.GetActiveCanvas().GetGrid();
    },
    [&](const Grid& grid){
      auto& canvas = app.GetActiveCanvas();
      canvas.SetGrid(grid);
      canvas.Refresh();
    }};

  auto getCanvas = [&app]()->Canvas&{return app.GetActiveCanvas();};

  // Bottom half, the selected color, palette and zoom controls.
  panels.color = std::make_unique<ColorPanel>(&frame,
    palette,
    pickPaint,
    getBg,
    gridAccessor,
    showGridDialog,
    std::make_unique<FaintFrameContext>(getCanvas),
    app.GetStatusInfo(),
    art);

  auto rows = make_wx<wxBoxSizer>(wxVERTICAL);
  rows->Add(row1, 1, wxEXPAND);
  rows->Add(panels.color->AsWindow(), 0, wxEXPAND);
  frame.SetSizer(rows);
  frame.Layout();

  app.SetTabCtrl(panels.tabControl.get()); // Fixme: Remove

  frame.SetMinSize(wxSize(640, 480));
}

static int suggested_save_format_index(const Formats& formats,
  const Optional<FilePath>& maybeOldFilePath,
  bool rasterOnly)
{
  // Gets the index for the suggested format for a file dialog for the
  // given file path.
  // If there's no old path, the index defaults to png or
  // svg depending on rasterOnly.

  auto png_svg_or_zero = [&](){
    return get_file_format_index(formats,
      FileExtension(rasterOnly ? "png" : "svg")).Or(0);
  };

  auto by_extension_or_default = [&](const FilePath& p){
    return get_file_format_index(formats, p.Extension()).Visit(
      [](int v){
        return v;
      },
      png_svg_or_zero);
  };

  return maybeOldFilePath.Visit(by_extension_or_default, png_svg_or_zero);
}

struct SaveDialogInfo{
  SaveDialogInfo(const FilePath& path, Format& format)
    : path(path), format(format)
  {}
  FilePath path;
  Format& format;
};

static Optional<SaveDialogInfo> show_save_as_dialog(wxWindow* parent,
  DialogContext& dialogContext,
  Canvas& canvas,
  const Formats& formats)
{
  Optional<FilePath> oldFilePath(canvas.GetFilePath());

  const wxFileName initialPath(oldFilePath.IsSet() ?
    wxFileName(to_wx(oldFilePath.Get().Str())) :
    wxFileName());

  auto saveFormats = saving_file_formats(formats);

  wxFileDialog dlg(parent,
    "Save as",
    initialPath.GetPath(),
    initialPath.GetName(),
    to_wx(file_dialog_filter(saveFormats)),
    wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

  dlg.SetFilterIndex(suggested_save_format_index(saveFormats,
    oldFilePath,
    canvas.GetObjects().empty()));

  const auto result = dialogContext.ShowModal(dlg);
  if (result != DialogChoice::OK){
    return {};
  }

  FilePath path(FilePath::FromAbsoluteWx(dlg.GetPath()));
  auto format = get_save_format(saveFormats,
    path.Extension(),
    dlg.GetFilterIndex());

  return format.Visit(
    [&](Format& f) -> Optional<SaveDialogInfo>{
      return {SaveDialogInfo(path, f)};
    },
    []() -> Optional<SaveDialogInfo>{
      // Fixme: Need an error message, though: Can this happen?
      // The dialog will either give the specified extension or
      // the selected extension if none matches.
      return {};
    });
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
// Fixme: Also move notifysaved out.
// ... leaves nothing though. :)
static bool save(wxWindow* parent,
  AppContext& app,
  Format& format,
  const FilePath& filePath,
  Canvas& canvas,
  bool backup=false)
{
  SaveResult result = format.Save(filePath, canvas);
  if (result.Failed()){
    show_error(parent, app, Title("Failed Saving"), result.ErrorDescription());
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
      frame(f),
      panels(std::move(in_panels)),
      pythonContext(python),
      state(std::move(in_state)),
      m_toolActions(app),
      windows(help, interpreter)
  {}

  FaintDialogContext& GetDialogContext(){
    return appContext.GetDialogContext();
  }

  FaintWindowContext& appContext;
  dumb_ptr<FaintFrame> frame;
  std::unique_ptr<FaintPanels> panels;
  PythonContext& pythonContext;
  std::unique_ptr<FaintState> state;
  FaintToolActions m_toolActions;
  FaintFloatingWindows windows;
  std::vector<std::unique_ptr<Cleaner>> cleanup;
};

static void select_tool(ToolId id, FaintState& state, FaintPanels& panels,
  AppContext& app, ToolActions& actions)
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
    ActiveCanvas(app),
    actions);
}

FaintWindow::FaintWindow(Art& art,
  const PaintMap& palette,
  HelpFrame* helpFrame,
  InterpreterFrame* interpreterFrame,
  bool silentMode,
  bool usePenTablet)
{
  auto frame = make_wx<FaintFrame>();

  auto panels = std::make_unique<FaintPanels>();
  auto state = std::make_unique<FaintState>(default_tool_settings(),
    silentMode, usePenTablet);

  // Fixme: Who deletes?
  FaintWindowContext* appContext = new FaintWindowContext(*this,
    art, create_faint_statusbar(frame), *helpFrame, *interpreterFrame);

  FaintWindowPythonContext* pythonContext = new
    FaintWindowPythonContext(*this, *interpreterFrame);

  initialize_panels(*frame, *appContext, *panels, art, palette);

  // Fixme: Wrong place for such stuff, has nothing todo with main
  // frame. Consider App.
  state->formats = built_in_file_formats();

  m_impl = std::make_unique<FaintWindowImpl>(frame,
    std::move(panels),
    std::move(state),
    *appContext,
    *pythonContext,
    *helpFrame,
    *interpreterFrame);

  events::on_add_to_palette(frame, [this](const Paint& paint){
    AddToPalette(paint);
  });

  events::on_request_close_faint(frame, [this](bool force){
    m_impl->frame->Close(force);
  });

  events::on_open_files(frame, [this](const FileList& files){
    Open(files);
  });

  events::on_tool_change(frame, [&](ToolId toolId){
    select_tool(toolId, *m_impl->state,
      *m_impl->panels, m_impl->appContext,
      m_impl->m_toolActions);
    UpdateShownSettings();
  });

  events::on_copy_color_string(frame, [&](const Color& color, CopyColorMode mode){
    Clipboard clipboard;
    if (!clipboard.Good()){
      show_copy_color_error(m_impl->frame.get(), m_impl->appContext);
      return;
    }
    clipboard.SetText(mode == CopyColorMode::HEX ?
      str_hex(color) :
      str_smart_rgba(color));
  });

  bind(frame, EVT_FAINT_SwapColors,
    [this](){
      auto& app(m_impl->appContext);
      auto oldFg = app.Get(ts_Fg);
      auto oldBg = app.Get(ts_Bg);
      app.Set(ts_Fg, oldBg);
      app.Set(ts_Bg, oldFg);
    });

  events::on_set_focus_entry(frame, [this](){
    if (m_impl->state->textEntryCount == 0){
      // Entry controls are numeric, not all shortcuts need to be
      // disabled
      const bool numeric = true;
      m_impl->panels->menubar->BeginTextEntry(numeric);
    }
    m_impl->state->textEntryCount++;
  });

  events::on_kill_focus_entry(frame, [this](){
    EndTextEntry();
  });

  events::on_canvas_modified_final(frame, [this](CanvasId canvasId){
    auto& panels(*m_impl->panels);
    auto& state(*m_impl->state);
    update_canvas_state(canvasId, panels, state);
    update_shown_settings(state, panels);
    m_impl->GetDialogContext().Reinitialize();
  });

  events::on_grid_modified(frame,
    [this](CanvasId canvasId){
      auto& panels(*m_impl->panels);
      if (canvasId == get_active_canvas(panels).GetId()){
        panels.color->UpdateGrid();
      }
    });

  events::on_switch_canvas(frame,
    [this](CanvasId canvasId){
      auto& panels(*m_impl->panels);
      auto& state(*m_impl->state);
      update_zoom(panels);
      panels.color->UpdateGrid();
      update_canvas_state(canvasId, panels, state);
      state.activeTool->SelectionChange();
      update_shown_settings(state, panels);
      m_impl->GetDialogContext().Reinitialize();
    });

  events::on_zoom_modified(frame,
    [this](CanvasId canvasId){
      auto& panels(*m_impl->panels);
      if (canvasId == get_active_canvas(panels).GetId()){
        update_zoom(panels);
      }
    });

  events::on_layer_change(frame, [&](Layer layer){
    auto& state(*m_impl->state);
    auto& panels(*m_impl->panels);
    state.layer = layer;
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
        bool quit = ask_exit_unsaved_changes(m_impl->frame.get(),
          m_impl->appContext);
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

  bind_fwd(toolPanel, EVT_FAINT_IntSettingChange,
    [this](const SettingEvent<IntSetting>& e){
      change_setting(*this, e.GetSetting(), e.GetValue(), from_control(true),
        m_impl->appContext.GetDialogContext());
    });

  bind_fwd(toolPanel, EVT_FAINT_FloatSettingChange,
    [this](const SettingEvent<FloatSetting>& e){
      change_setting(*this, e.GetSetting(), e.GetValue(), from_control(true),
        m_impl->appContext.GetDialogContext());
    });

  bind_fwd(toolPanel, EVT_FAINT_BoolSettingChange,
    [this](const SettingEvent<BoolSetting>& e){
      change_setting(*this, e.GetSetting(), e.GetValue(), from_control(true),
        m_impl->appContext.GetDialogContext());
    });

  bind_fwd(toolPanel, EVT_FAINT_StringSettingChange,
    [this](const SettingEvent<StringSetting>& e){
      change_setting(*this, e.GetSetting(), e.GetValue(), from_control(true),
        m_impl->appContext.GetDialogContext());
    });

  bind_fwd(toolPanel,EVT_FAINT_SettingsChange,
    [this](const SettingsEvent& e){
      // Note that from_control(...) is not used when changing multiple
      // settings. This is because the font dialog can change the font
      // size, which is also shown in another control which must be
      // updated, so there's no point in trying to not refresh controls.
      change_settings(*this, e.GetSettings());
    });

  bind_fwd(m_impl->panels->color->AsWindow(), EVT_FAINT_PaintSettingChange,
    [this](const SettingEvent<PaintSetting>& e){
      change_setting(*this, e.GetSetting(), e.GetValue(), from_control(false),
        m_impl->appContext.GetDialogContext());
    });

  m_impl->panels->tool->SelectTool(ToolId::LINE);
  restore_persisted_state(m_impl->frame.get(), storage_name("FaintWindow"));
}

FaintWindow::~FaintWindow(){
  // This destructor must be defined otherwise unique_ptr to
  // forward-declared FaintWindowImpl won't compile.
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
  events::queue_request_close_faint(*m_impl->frame, force);
}

void FaintWindow::CloseDocument(Canvas& canvas, bool force){
  FaintPanels& panels(*m_impl->panels);
  auto& tabControl = *(panels.tabControl);
  for (auto i : up_to(tabControl.GetCanvasCount())){
    Canvas& other = tabControl.GetCanvas(i)->GetInterface();
    if (&other == &canvas){
      // Freeze the color panel to avoid the FrameControl refreshing
      // during page close on GTK. (See issue 122).
      auto freezer = freeze(panels.color);
      tabControl.Close(i, force);
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
  else{
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

Canvas& FaintWindow::GetCanvas(const CanvasId& id){
  CanvasPanel* c = m_impl->panels->tabControl->GetCanvas(id);
  assert(c != nullptr);
  return c->GetInterface();
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
  auto& state = (*m_impl->state);
  // Fixme: Add common tablet init which is no-op on Linux (for now)
  if (!state.silentMode && state.usePenTablet){
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
      tablet::show_tablet_error_message(m_impl->frame.get(),
        m_impl->appContext,
        tabletResult);
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
  canvas->SetGrid(m_impl->appContext.GetDefaultGrid());
  return canvas->GetInterface();
}

void FaintWindow::NextTab(){
  m_impl->panels->tabControl->SelectNext();
}

void FaintWindow::Open(const FileList& paths){
  if (paths.empty()){
    return;
  }

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
      else{
        notFound.push_back(filePath); // Fixme
      }
    }
  }
  catch (const BitmapOutOfMemory&){
    show_error(m_impl->frame.get(),
      m_impl->appContext,
      Title("Out of memory"),
      "Insufficient memory to load all images.");
  }

  if (get_canvas_count(panels) == 0){
    NewDocument(m_impl->appContext.GetDefaultImageInfo());
  }

  if (notFound.size() == 1){
    show_file_not_found_error(m_impl->frame.get(),
      m_impl->appContext,
      notFound.back());
  }
  else if (notFound.size() > 1){
    utf8_string error = "Files not found: \n";
    for (const FilePath& path : notFound){
      error += path.Str() + "\n";
    }
    show_error(m_impl->frame.get(), m_impl->appContext,
      Title("Files not found"),
      error);
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
            m_impl->appContext,
            filePath,
            props.GetError());
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
          show_load_warnings(m_impl->frame.get(),
            m_impl->appContext,
            props);
        }
      }
      return &(newCanvas->GetInterface());
    }
  }
  if (!state.silentMode){
    show_file_not_supported_error(m_impl->frame.get(),
      m_impl->appContext,
      filePath);
  }
  return nullptr;
}

void FaintWindow::PreviousTab(){
  m_impl->panels->tabControl->SelectPrevious();
}

void FaintWindow::QueueLoad(const FileList& files){
  events::queue_open_files(*m_impl->frame, files);
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

  auto update_recent =
    [&](const Format& f, const FilePath& p){
    if (f.CanLoad() || has_load_format(state.formats, p.Extension())){
      m_impl->panels->menubar->AddRecentFile(p);
    }
  };

  // Fixme: Tidy up. ;_;
  return maybeFilePath.Visit(
    [&](const FilePath& p){
      return get_save_format(state.formats, p.Extension()).Visit(
        [&](Format& f){
          bool savedOk = save(m_impl->frame.get(), m_impl->appContext,
            f, p, canvas);
          if (savedOk){
            update_recent(f, p);
          }
          return savedOk;
        },
        [&](){
          return ShowSaveAsDialog(canvas);
        });
    },
    [&](){
      return show_save_as_dialog(m_impl->frame.get(),
        m_impl->GetDialogContext(),
        canvas,
        state.formats).Visit(
          [&](const SaveDialogInfo& info){
            const bool savedOk = save(m_impl->frame.get(),
              m_impl->appContext,
              info.format,
              info.path,
              canvas,
              false);
            if (savedOk){
              update_recent(info.format, info.path);
            }
            return savedOk;
          },
          otherwise(false));
    });
}

bool FaintWindow::ShowSaveAsDialog(Canvas& canvas, bool backup){
  const auto& formats = m_impl->state->formats;
  const auto saveResult = show_save_as_dialog(m_impl->frame.get(),
    m_impl->appContext.GetDialogContext(),
    canvas,
    formats);

  // Fixme: Duplicates FaintWindow::Save()
  return saveResult.Visit(
    [&](const SaveDialogInfo& info){
      const bool savedOk =
        save(m_impl->frame.get(),
          m_impl->appContext,
          info.format,
          info.path,
          canvas,
          backup);
      if (!savedOk){
        return false;
      }
      const bool canLoad = info.format.CanLoad() ||
        has_load_format(formats, info.path.Extension());
      if (canLoad){
        m_impl->panels->menubar->AddRecentFile(info.path);
      }
      return true;
    },
    otherwise(false));
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
  else{
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
