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

#include "wx/config.h" // wxConfig, for recent files
#include "wx/filehistory.h" // wxFileHistory, for recent files
#include "wx/filename.h" // For recent files
#include "wx/frame.h"
#include "wx/menu.h"
#include "app/canvas.hh"
#include "app/context-commands.hh"
#include "app/cut-and-paste.hh"
#include "app/faint-resize-dialog-context.hh"
#include "gui/dialogs.hh"
#include "gui/events.hh"
#include "gui/interpreter-frame.hh"
#include "gui/menu-bar.hh"
#include "gui/menu-predicate.hh"
#include "util-wx/bind-event.hh"
#include "util-wx/convert-wx.hh"
#include "util/command-util.hh"
#include "util/frame-props.hh"
#include "util/index-iter.hh"
#include "util/zoom-level.hh"

namespace faint{

class RecentFilesImpl : public RecentFiles {
  // Handles the storing, retrieving and menu-states for
  // recent files in the Menubar.
public:
  // The recent file menu receives the list of files,
  // the wxMenuBar is used to modify related items.
  // in that menu,
  RecentFilesImpl(wxMenu* recentFilesMenu, int openAllId, int clearId)
    : m_clearId(clearId),
      m_config("Faint"),
      m_fileHistory(),
      m_menu(recentFilesMenu),
      m_openAllId(openAllId)
  {
    m_fileHistory.UseMenu(recentFilesMenu);
    Load();
  }

  void Add(const FilePath& path) override{
    m_fileHistory.AddFileToHistory(to_wx(path.Str()));
    UpdateMenu();
  }

  void Clear() override{
    m_cleared.clear();
    for (size_t i = 0; i != m_fileHistory.GetCount(); i++){
      m_cleared.push_back(utf8_string(m_fileHistory.GetHistoryFile(i)));
    }
    for (size_t i = m_fileHistory.GetCount(); i != 0; i--){
      m_fileHistory.RemoveFileFromHistory(i - 1);
    }
    Save();
    UpdateMenu();
  }

  FilePath Get(size_t i) const override{
    wxString strPath(m_fileHistory.GetHistoryFile(i));
    return FilePath::FromAbsoluteWx(strPath);
  }

  void Save() override{
    m_fileHistory.Save(m_config);
  }

  size_t Size() const override{
    return m_fileHistory.GetCount();
  }

  void UndoClear() override{
    if (m_fileHistory.GetCount() != 0){
      return;
    }
    for (size_t i = 0; i != m_cleared.size(); i++){
      m_fileHistory.AddFileToHistory(to_wx(m_cleared[i]));
    }
    UpdateMenu();
  }
private:
  void Load(){
    m_fileHistory.Load(m_config);
    m_cleared.clear();
    for (size_t i = m_fileHistory.GetCount(); i != 0; i--){
      wxString str = m_fileHistory.GetHistoryFile(i-1);
      if (!wxFileName(str).FileExists()){
        m_fileHistory.RemoveFileFromHistory(i - 1);
      }
    }
    UpdateMenu();
  }

  void UpdateMenu(){
    bool hasFiles = m_fileHistory.GetCount() != 0;

    if (hasFiles){
      m_menu->Enable(m_openAllId, true);
      m_menu->Enable(m_clearId, true);
      m_menu->SetLabel(m_clearId, "&Clear");
      m_menu->SetHelpString(m_clearId, "Clear Recent Files");
    }
    else {
      m_menu->Enable(m_openAllId, false);
      if (!m_cleared.empty()){
        m_menu->Enable(m_clearId, true);
        m_menu->SetLabel(m_clearId, "Undo &Clear");
        m_menu->SetHelpString(m_clearId, "Undo clearing Recent Files");
      }
      else {
        m_menu->Enable(m_clearId, false);
        m_menu->SetLabel(m_clearId, "&Clear");
        m_menu->SetHelpString(m_clearId, "Clear Recent Files");
      }
    }
  }
  std::vector<utf8_string> m_cleared;
  const int m_clearId;
  wxConfig m_config;
  wxFileHistory m_fileHistory;
  wxMenu* m_menu;
  const int m_openAllId;
};

static FileList get_all(const RecentFiles& recent){
  FileList files;
  for (size_t i = 0; i != recent.Size(); i++){
    files.push_back(recent.Get(i));
  }
  return files;
}

// Wrapper to allow the MenuPredicate to be unaware of wxMenu.
class MenuRefWX : public MenuReference{
public:
  MenuRefWX(wxMenuBar* menu)
    : m_menu(menu)
  {}

  void Append(wxMenu* menu, const wxString& title){
    m_menu->Append(menu, title);
  }

  void Enable(int itemId, bool enable) override{
    m_menu->Enable(itemId, enable);
  }

  wxMenuBar* m_menu;
};

static utf8_string undo_label(const utf8_string& cmd){
  if (cmd.empty()){
    return "&Undo\tCtrl+Z";
  }
  return "&Undo " + cmd + "\tCtrl+Z";
}

static utf8_string redo_label(const utf8_string& cmd){
  if (cmd.empty()){
    return "&Redo\tCtrl+Y";
  }
  return "&Redo " + cmd + "\tCtrl+Y";
}

static void clear_or_undo(RecentFiles& recent){
  if (recent.Size() != 0){
    recent.Clear();
  }
  else {
    recent.UndoClear();
  }
}

static void harmonize_zoom(const Canvas& source, AppContext& app){
  const ZoomLevel zoom(source.GetZoomLevel());
  for (auto i : up_to(app.GetCanvasCount())){
    Canvas& canvas = app.GetCanvas(i);
    if (canvas.GetId() != source.GetId()){
      canvas.SetZoom(zoom);
    }
  }
}

enum ToggleShortcut{
  TOGGLE_NEVER, // Never disable this shortcut
  TOGGLE_ALPHABETIC, // Disable this shortcut during alphabetic entry,
                     // but keep it for numeric entry
  TOGGLE_ALL // Disable this shortcut when any entry field has focus
};

class Label{
public:
  Label(const utf8_string& label, ToggleShortcut toggle=TOGGLE_NEVER)
    : m_label(to_wx(label)),
      m_toggle(toggle)
  {}

  Label(const utf8_string& label,
    const utf8_string& help,
    ToggleShortcut toggle=TOGGLE_NEVER)
    : m_label(to_wx(label)),
      m_help(to_wx(help)),
      m_toggle(toggle)
  {}

  wxString GetLabelText() const{
    return m_label;
  }

  wxString GetHelpText() const{
    return m_help;
  }

  ToggleShortcut GetToggle() const{
    return m_toggle;
  }
private:
  wxString m_label;
  wxString m_help;
  ToggleShortcut m_toggle;
};

class ToggleLabel{
public:
  ToggleLabel(int id, const wxString& label, bool disableNumeric)
    : m_id(id),
      m_disableNumeric(disableNumeric)
  {
    size_t tabPos = label.find("\t");
    if (tabPos != wxString::npos){
      m_label = label.substr(0,tabPos);
      m_shortcut = label.substr(tabPos);
    }
  }

  void EnableShortcut(wxMenuBar* mb) const{
    mb->SetLabel(m_id, m_label + m_shortcut);
  }

  void DisableShortcut(wxMenuBar* mb, bool numeric) const{
    if (!numeric || m_disableNumeric){
      mb->SetLabel(m_id, m_label);
    }
  }


private:
  int m_id;
  wxString m_label;
  wxString m_shortcut;
  bool m_disableNumeric;
};

class Menubar::MenubarImpl{
public:
  void Initialize(wxFrame& frame, AppContext& app, const ArtContainer& art){
    m_menuRef = new MenuRefWX(new wxMenuBar());
    wxMenu* fileMenu = new wxMenu();
    Add(fileMenu, wxID_NEW, Label("New\tCtrl+N", "Create a new image"),
      [&](){app.NewDocument(app.GetDefaultImageInfo());});

    Add(fileMenu, wxID_OPEN, Label("&Open...\tCtrl+O", "Open an existing image"),
      [&](){app.DialogOpenFile();});

    wxMenu* recent = new wxMenu();
    m_recentFilesMenu = recent;

    int openAllRecentId = Add(recent,
      Label("Open &All", "Open all recently used files"),
      [&](){app.Load(get_all(GetRecentFiles()));});
    int clearRecentId = Add(recent,
      Label("&Clear", "Clear the recent files list"),
      [&](){clear_or_undo(GetRecentFiles());});

    // The events for recent files is bound separately (as the menu
    // items are added dynamically
    // Fixme: Add e.g. bind_range to bind-event.hh, instead of using .Bind
    bind_menu(m_eventHandler, ClosedIntRange(wxID_FILE1, wxID_FILE1 + 10),
      [&](int eventId){
        int index = eventId - wxID_FILE1;
        FilePath filePath(GetRecentFiles().Get(to_size_t(index)));
        app.Load(filePath, change_tab(true));
    });

    fileMenu->AppendSubMenu(recent, "Open &Recent", "Recent files");
    fileMenu->AppendSeparator();
    Add(fileMenu, wxID_SAVE, Label("&Save\tCtrl+S", "Save the active image"),
      MenuPred(MenuPred::DIRTY),
      [&](){app.Save(app.GetActiveCanvas());});

    Add(fileMenu, wxID_SAVEAS,
      Label("Save &As...\tCtrl+Shift+S", "Save the active image with a new name"),
      [&](){
        app.DialogSaveAs(app.GetActiveCanvas(), false);
      });

    fileMenu->AppendSeparator();

    Add(fileMenu, wxID_EXIT, Label("E&xit\tAlt+F4", "Exit Faint"),
      [&](){
        app.Quit();
      });
    m_menuRef->Append(fileMenu, "&File");

    wxMenu* editMenu = new wxMenu();
    Add(editMenu, wxID_UNDO,
      Label(undo_label(""), "Undo the last action"),
      MenuPred(MenuPred::CAN_UNDO),
      [&](){
        if (m_undoing || m_redoing){
          return;
        }
        m_undoing = true;
        Canvas& active = app.GetActiveCanvas();
        active.Undo();
        active.Refresh();
        m_undoing = false;
      });

    Add(editMenu, wxID_REDO,
      Label(redo_label(""), "Redo the last undone action"),
      MenuPred(MenuPred::CAN_REDO),
      [&](){
        if (m_undoing || m_redoing){
          return;
        }
        m_redoing = true;
        Canvas& active = app.GetActiveCanvas();
        active.Redo();
        active.Refresh();
        m_redoing = false;
      });

    editMenu->AppendSeparator();

    Add(editMenu, wxID_CUT, Label("Cu&t\tCtrl+X","Cut selection"),
      MenuPred(MenuPred::ANY_SELECTION),
      [&](){cut_selection(app);});

    Add(editMenu, wxID_COPY, Label("&Copy\tCtrl+C","Copy selection"),
      MenuPred(MenuPred::ANY_SELECTION),
      [&](){copy_selection(app);});

    Add(editMenu, wxID_PASTE,
      Label("&Paste\tCtrl+V", "Paste from clipboard"),
      [&](){paste_to_active_canvas(app);});

    Add(editMenu, Label("Paste N&ew\tCtrl+Alt+V", "Paste to a new image"),
      [&](){paste_to_new_image(app);});

    editMenu->AppendSeparator();
    Add(editMenu, Label("Cr&op\tCtrl+Alt+C"),
      [&](){
        Canvas& active = app.GetActiveCanvas();
        active.RunCommand(context_crop(active));
      });

    Add(editMenu, wxID_DELETE, Label("&Delete\tDel"),
      // Fixme: Should be disabled on numeric entry, to allow using
      // delete in line width edit field (et. al.), but that ruins the
      // use of the delete key in the edit tool
      MenuPred(MenuPred::ANY_SELECTION),
      [&](){context_delete(app.GetActiveCanvas(),
        app.GetToolSettings().Get(ts_Bg));});

    editMenu->AppendSeparator();

    Add(editMenu, Label("Select &All\tCtrl+A"),
      [&](){
        Canvas& active = app.GetActiveCanvas();
        active.RunCommand(context_select_all(active));
      });

    Add(editMenu, Label("Select &None\tCtrl+D"),
      MenuPred(MenuPred::ANY_SELECTION),
      [&](){
        Canvas& active = app.GetActiveCanvas();
        active.RunCommand(context_deselect(active));
      });
    editMenu->AppendSeparator();

    Add(editMenu, Label("Python &Interpreter...\tF8"),
      [&](){app.TogglePythonConsole();});

    m_menuRef->Append(editMenu, "&Edit");

    wxMenu* viewMenu = new wxMenu();
    Add(viewMenu, EVT_FAINT_ZOOM_IN,
      Label("Zoom In\t+", "Zoom in one step", TOGGLE_ALPHABETIC),
      [&](){app.GetActiveCanvas().ZoomIn();});

    Add(viewMenu, EVT_FAINT_ZOOM_OUT,
      Label("Zoom Out\t-", "Zoom out one step", TOGGLE_ALPHABETIC),
      [&](){app.GetActiveCanvas().ZoomOut();});

    Add(viewMenu, FAINT_ZOOM_100_TOGGLE,
      Label("Zoom 1:1\t*", TOGGLE_ALPHABETIC),
      [&](){
        Canvas& active = app.GetActiveCanvas();
        if (active.GetZoomLevel().At100()){
          active.ZoomFit();
        }
        else{
          active.ZoomDefault();
        }
      });

    viewMenu->AppendSeparator();

    AddCheck(viewMenu,
      Label("&Tool Panel", "Show or hide the tool panel"), true,
      [&](bool checked){app.ShowToolPanel(checked);});

    AddCheck(viewMenu,
      Label("&Status Bar", "Show or hide the status bar"), true,
      [&](bool checked){app.ShowStatusbar(checked);});

    AddCheck(viewMenu,
      Label("&Color Panel", "Show or hide the color panel"), true,
      [&](bool checked){app.ShowColorPanel(checked);});

    viewMenu->AppendSeparator();

    Add(viewMenu, Label("&Maximize\tAlt+Enter"),
      [&](){app.ToggleMaximize();});

    Add(viewMenu, Label("&Fullscreen\tF11"),
      [&](){app.ToggleFullScreen(!app.IsFullScreen());});

    m_menuRef->Append(viewMenu, "&View");

    wxMenu* objectMenu = new wxMenu();
    Add(objectMenu,
      Label("&Group Objects\tCtrl+G", "Combine selected objects into a group"),
      MenuPred(MenuPred::MULTIPLE_SELECTED),
      [&](){
        Canvas& active = app.GetActiveCanvas();
        active.RunCommand(group_selected_objects(active));
      });

    Add(objectMenu,
      Label("&Ungroup Objects\tCtrl+U", "Disband selected group"),
      MenuPred(MenuPred::GROUP_IS_SELECTED),
      [&](){
        Canvas& active = app.GetActiveCanvas();
        active.RunCommand(ungroup_selected_objects(active));
      });
    objectMenu->AppendSeparator();

    Add(objectMenu,
      Label("Move &Forward\tF", TOGGLE_ALPHABETIC),
      MenuPred(MenuPred::CAN_MOVE_FORWARD),
      [&](){
        Canvas& active = app.GetActiveCanvas();
        active.RunCommand(context_objects_forward(active));
      });

    Add(objectMenu,
      Label("Move &Backward\tB", TOGGLE_ALPHABETIC),
      MenuPred(MenuPred::CAN_MOVE_BACKWARD),
      [&](){
        Canvas& active = app.GetActiveCanvas();
        active.RunCommand(context_objects_backward(active));
      });

    Add(objectMenu,
      Label("Bring to Front\tCtrl+F"),
      MenuPred(MenuPred::CAN_MOVE_FORWARD),
      [&](){
        Canvas& active = app.GetActiveCanvas();
        active.RunCommand(context_objects_to_front(active));
      });

    Add(objectMenu, NewId(),
      Label("Move to Back\tCtrl+B", TOGGLE_ALPHABETIC),
      MenuPred(MenuPred::CAN_MOVE_BACKWARD),
      [&](){
        Canvas& active = app.GetActiveCanvas();
        active.RunCommand(context_objects_to_back(active));
      });

    objectMenu->AppendSeparator();

    Add(objectMenu,
      Label("Become &Path", "Turn the selected objects into Path objects"),
      MenuPred(MenuPred::OBJECT_SELECTION),
      [&](){
        Canvas& active = app.GetActiveCanvas();
        active.RunCommand(context_objects_to_path(active));
      });

    Add(objectMenu,
      Label("Flatten (Rasterize)\tCtrl+Space",
        "Flatten selected objects onto the background"),
      MenuPred(MenuPred::HAS_OBJECTS),
      [&](){
        Canvas& active = app.GetActiveCanvas();
        active.RunCommand(context_flatten(active));
      });

    m_menuRef->Append(objectMenu, "&Objects");

    wxMenu* imageMenu = new wxMenu();
    AddFull(imageMenu,
      Label("&Flip/Rotate...\tCtrl+R", "Flip or rotate the image or selection"),
      bind_show_rotate_dialog(art, app.GetDialogContext()), app);

    Add(imageMenu,
      Label("&Resize...\tCtrl+E", "Scale or resize the image or selection"),
      [&](){
        show_resize_dialog(app);
      });

    imageMenu->AppendSeparator();

    wxMenu* colorMenu = new wxMenu();

    imageMenu->AppendSubMenu(colorMenu, "&Color", "Color");

    Add(colorMenu, Label("&Invert", "Invert the colors"),
      [&](){
        auto& active(app.GetActiveCanvas());
        active.RunCommand(context_targetted(get_invert_command(), active));
      });

    Add(colorMenu, Label("&Desaturate", "Replace colors with grays"),
      [&](){
        auto& active(app.GetActiveCanvas());
        active.RunCommand(context_targetted(get_desaturate_weighted_command(),
          active));
      });

    colorMenu->AppendSeparator();

    Add(colorMenu,
      Label("&Alpha...", "Adjust alpha transparency"),
      [&](){
        show_alpha_dialog(app.GetDialogContext());
      });

    Add(colorMenu,
      Label("&Brightness/Contrast...",
        "Adjust the image brightness and contrast"),
      show_brightness_contrast_dialog, app);


    Add(colorMenu, Label("&Threshold...", "Threshold the image by intensity"),
      [&](){
        show_threshold_dialog(app.GetDialogContext(), app.GetToolSettings());
      });

    Add(colorMenu,
      Label("&Color balance...", "Adjust individual colors)"),
      show_color_balance_dialog, app);

    wxMenu* effectsMenu = new wxMenu();
    imageMenu->AppendSubMenu(effectsMenu, "&Effects", "Effects");

    Add(effectsMenu, Label("&Sharpness...", "Sharpen or blur the image"),
      [&](){
        app.Modal(show_sharpness_dialog);
      });

    Add(effectsMenu, Label("&Pinch/Whirl...", "Apply Pinch or Whirl filter"),
      show_pinch_whirl_dialog, app);

    Add(effectsMenu, Label("P&ixelize...", "Pixelize the image"),
      show_pixelize_dialog, app);

    m_menuRef->Append(imageMenu, "&Image");

    wxMenu* tabMenu = new wxMenu();
    Add(tabMenu, wxID_FORWARD, Label("&Next Tab\tCtrl+Tab"),
      [&](){app.NextTab();});

    Add(tabMenu, wxID_BACKWARD, Label("&Previous Tab\tCtrl+Shift+Tab"),
      [&](){app.PreviousTab();});

    Add(tabMenu, wxID_REMOVE, Label("&Close\tCtrl+W"),
      [&](){app.Close(app.GetActiveCanvas());});
    m_menuRef->Append(tabMenu, "&Tabs");

    wxMenu* helpMenu = new wxMenu();
    Add(helpMenu, wxID_HELP,
      Label("&Help Index\tF1", "Show the help for Faint"),
      [&](){app.ToggleHelpFrame();});

    helpMenu->AppendSeparator();

    Add(helpMenu, wxID_ABOUT,
      Label("&About", "Information about Faint"),
      [&](){show_faint_about_dialog(&frame,
          app.GetDialogContext());});

    m_menuRef->Append(helpMenu, "&Help");

    frame.SetMenuBar(m_menuRef->m_menu);

    m_recentFiles = new RecentFilesImpl(m_recentFilesMenu,
      openAllRecentId,
      clearRecentId);

    // Events from the zoom control
    bind_menu(m_eventHandler, FAINT_ZOOM_100,
      [&](){app.GetActiveCanvas().ZoomDefault();});

    bind_menu(m_eventHandler, FAINT_ZOOM_IN_ALL,
      [&](){
        Canvas& active = app.GetActiveCanvas();
        active.ZoomIn();
        harmonize_zoom(active, app);
      });

    bind_menu(m_eventHandler, FAINT_ZOOM_OUT_ALL,
      [&](){
        Canvas& active = app.GetActiveCanvas();
        active.ZoomOut();
        harmonize_zoom(active, app);
      });

    bind_menu(m_eventHandler, FAINT_ZOOM_100_ALL,
      [&](){
        for (auto i : up_to(app.GetCanvasCount())){
          app.GetCanvas(i).ZoomDefault();
        }
      });

    bind_menu(m_eventHandler, FAINT_ZOOM_FIT,
      [&](){
        app.GetActiveCanvas().ZoomFit();
      });

    bind_menu(m_eventHandler, FAINT_ZOOM_FIT_ALL,
      [&](){
        for (auto i : up_to(app.GetCanvasCount())){
          app.GetCanvas(i).ZoomFit();
        }
      });
  }

  MenubarImpl(wxFrame& frame, AppContext& app, const ArtContainer& art)
    : m_entryMode(EntryMode::NONE),
      m_eventHandler(*frame.GetEventHandler()),
      m_maxId(wxID_HIGHEST+1),
      m_menuRef(nullptr),
      m_recentFiles(nullptr),
      m_recentFilesMenu(nullptr),
      m_redoing(false),
      m_undoing(false)
  {
    Initialize(frame, app, art);
  }

  ~MenubarImpl(){
    delete m_menuRef;
    delete m_recentFiles;
  }

  void AddShortcutDisable(int id, const Label& label){
    ToggleShortcut toggle = label.GetToggle();

    bool disableOnEntry = (toggle != TOGGLE_NEVER);
    if (disableOnEntry){
      bool disableNumeric = (toggle == TOGGLE_ALL);
      m_notWhileTexting.push_back(ToggleLabel(id, label.GetLabelText(),
        disableNumeric));
    }
  }

  int Add(wxMenu* menu, const Label& label,
    const std::function<void()>& func)
  {
    int id = NewId();
    Add(menu, id, label, func);
    return id;
  }

  int Add(wxMenu* menu,
    const Label& label,
    const bmp_dialog_func& show_dialog,
    AppContext& app)
  {
    return Add(menu, label,
      [=,&app](){
        app.Modal(show_dialog);
      });
  }

  int AddFull(wxMenu* menu,
    const Label& label,
    const dialog_func& show_dialog,
    AppContext& app)
  {
    return Add(menu, label,
      [=,&app](){
        app.ModalFull(show_dialog);
      });
  }

  void Add(wxMenu* menu,
    int id,
    const Label& label,
    const std::function<void()>& func)
  {
    menu->Append(id, label.GetLabelText(), label.GetHelpText());
    AddShortcutDisable(id, label);
    bind_menu(m_eventHandler, id, func);
  }

  int Add(wxMenu* menu,
    const Label& label,
    const MenuPred& predicate,
    const std::function<void()>& func)
  {
    int id = NewId();
    Add(menu, id, label, predicate, func);
    return id;
  }

  void Add(wxMenu* menu,
    int id,
    const Label& label,
    const MenuPred& predicate,
    const std::function<void()>& func)
  {
    menu->Append(id, label.GetLabelText(), label.GetHelpText());
    AddShortcutDisable(id, label);
    m_predicates.push_back(predicate.GetBound(id));
    bind_menu(m_eventHandler, id, func);
  }

  void AddCheck(wxMenu* menu,
    const Label& label,
    bool checked,
    const std::function<void(bool)>& func)
  {
    int id = NewId();
    menu->AppendCheckItem(id, label.GetLabelText(), label.GetHelpText());
    AddShortcutDisable(id, label);
    bind_menu_check(m_eventHandler, id, func);
    menu->Check(id, checked);
  }

  void Update(const MenuFlags& flags){
    m_menuRef->m_menu->SetLabel(wxID_UNDO, to_wx(undo_label(flags.undoLabel)));
    m_menuRef->m_menu->SetLabel(wxID_REDO, to_wx(redo_label(flags.redoLabel)));
    for (BoundMenuPred& predicate : m_predicates){
      predicate.Update(*m_menuRef, flags);
    }
  }

  void UpdateZoom(const ZoomLevel& zoom){
    m_menuRef->Enable(FAINT_ZOOM_IN, !zoom.AtMax());
    m_menuRef->Enable(FAINT_ZOOM_OUT, !zoom.AtMin());
    if (zoom.At100()){
      m_menuRef->m_menu->SetLabel(FAINT_ZOOM_100_TOGGLE, "Zoom Fit\t*");
      m_menuRef->m_menu->SetHelpString(FAINT_ZOOM_100_TOGGLE,
        "Fit image in view");
    }
    else {
      m_menuRef->m_menu->SetLabel(FAINT_ZOOM_100_TOGGLE, "Zoom 1:1\t*");
      m_menuRef->m_menu->SetHelpString(FAINT_ZOOM_100_TOGGLE,
        "Zoom to actual size");
    }
  }

  void BeginTextEntry(bool numericOnly){
    for (ToggleLabel& label : m_notWhileTexting){
      label.DisableShortcut(m_menuRef->m_menu, numericOnly);
    }
    m_entryMode = numericOnly ? EntryMode::NUMERIC : EntryMode::ALPHA_NUMERIC;
  }

  void EndTextEntry(){
    for (ToggleLabel& label : m_notWhileTexting){
      label.EnableShortcut(m_menuRef->m_menu);
    }
    m_entryMode = EntryMode::NONE;
  }

  RecentFiles& GetRecentFiles(){
    return *m_recentFiles;
  }

  EntryMode GetTextEntryMode() const{
    return m_entryMode;
  }

  int NewId(){
    return m_maxId++;
  }

private:
  EntryMode m_entryMode;
  wxEvtHandler& m_eventHandler;
  int m_maxId;
  MenuRefWX* m_menuRef;
  std::vector<ToggleLabel> m_notWhileTexting;
  std::vector<BoundMenuPred> m_predicates;
  RecentFiles* m_recentFiles;
  wxMenu* m_recentFilesMenu;
  bool m_redoing;
  bool m_undoing;
};

Menubar::Menubar(wxFrame& frame, AppContext& app, const ArtContainer& art)
  : m_impl(nullptr)
{
  m_impl = new MenubarImpl(frame, app, art);
}

Menubar::~Menubar(){
  delete m_impl;
}

void Menubar::BeginTextEntry(bool numericOnly){
  m_impl->BeginTextEntry(numericOnly);
}

void Menubar::EndTextEntry(){
  m_impl->EndTextEntry();
}

EntryMode Menubar::GetTextEntryMode() const{
  return m_impl->GetTextEntryMode();
}

void Menubar::Update(const MenuFlags& flags){
  m_impl->Update(flags);
}

void Menubar::UpdateZoom(const ZoomLevel& zoomLevel){
  m_impl->UpdateZoom(zoomLevel);
}

void Menubar::StoreRecentFiles(){
  m_impl->GetRecentFiles().Save();
}

void Menubar::AddRecentFile(const FilePath& p){
  m_impl->GetRecentFiles().Add(p);
}

} // namespace
