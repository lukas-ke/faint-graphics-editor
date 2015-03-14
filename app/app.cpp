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

#include <algorithm>
#include <cctype>
#include <sstream>
#include "wx/app.h"
#include "wx/cmdline.h"
#include "wx/filename.h"
#include "wx/frame.h"
#include "app/get-art-container.hh"
#include "app/one-instance.hh"
#include "app/write-exception-log.hh"
#include "bitmap/color.hh"
#include "bitmap/paint.hh"
#include "generated/resource/load-resources.hh"
#include "gui/art-container.hh"
#include "gui/faint-window.hh"
#include "gui/help-frame.hh"
#include "gui/interpreter-frame.hh"
#include "python/py-initialize-ifaint.hh"
#include "python/py-interface.hh"
#include "python/py-exception.hh"
#include "python/py-key-press.hh"
#include "python/python-context.hh"
#include "text/formatting.hh"
#include "util/optional.hh"
#include "util/paint-map.hh"
#include "util/settings.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/file-path-util.hh"
#include "util-wx/gui-util.hh"
#include "util-wx/key-codes.hh"

namespace faint{

static utf8_string get_default_faint_port(){
  return utf8_string("3793");
}

struct CommandLine{
  CommandLine() :
    forceNew(false),
    preventServer(false),
    silentMode(false),
    script(false),
    port(get_default_faint_port())
  {}
  bool forceNew; // single instance
  bool preventServer; // single instance
  bool silentMode;
  bool script;
  Optional<FilePath> scriptPath;
  utf8_string port;
  FileList files;
  utf8_string arg; // Script argument
};

static const wxCmdLineEntryDesc g_cmdLineDesc[] = {
  {wxCMD_LINE_SWITCH, "h", "help",
    "Displays help on the command line parameters",
   wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP},

  {wxCMD_LINE_SWITCH, "s", "silent",
   "Disables the GUI. Requires specifying a script with with --run.",
   wxCMD_LINE_VAL_STRING,
   wxCMD_LINE_PARAM_OPTIONAL},

  {wxCMD_LINE_PARAM, "e", "whatev", "Image files", wxCMD_LINE_VAL_STRING,
   wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE},

  {wxCMD_LINE_SWITCH, "i", "newinst", "Force new instance",
   wxCMD_LINE_VAL_STRING,
   wxCMD_LINE_PARAM_OPTIONAL},

  {wxCMD_LINE_SWITCH, "ii", "noserver",
   "Prevent this instance from becoming a main app", wxCMD_LINE_VAL_STRING,
   wxCMD_LINE_PARAM_OPTIONAL},

  {wxCMD_LINE_OPTION, "", "port",
   "Specify port used for IPC.", wxCMD_LINE_VAL_STRING,
   wxCMD_LINE_PARAM_OPTIONAL},

  {wxCMD_LINE_OPTION, "", "run",
   "Run a Python script file after loading images",
   wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL},

  {wxCMD_LINE_OPTION, "arg", "arg",
   "Custom argument stored in ifaint.cmd_arg.", // Fixme: Duplication
   wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL},

  {wxCMD_LINE_NONE, "", "", "",
   wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL} // Sentinel
};

static utf8_string get_string(const wxCmdLineParser& parser,
  const std::string& name,
  const utf8_string& defaultStr=utf8_string(""))
{
  wxString str;
  parser.Found(name, &str);
  if (str.empty()){
    return defaultStr;
  }
  return to_faint(str);
}

static bool valid_port(const std::string& str){
  if (std::all_of(begin(str), end(str), [](auto v){return std::isdigit(v);})){
    std::stringstream ss(str);
    int i = 0;
    ss >> i;
    return 0 <= i && i <= 65535;
  }
  return false;
}

utf8_string format_run_script_error(const FilePath& path,
  const FaintPyExc& err)
{
  return space_sep("Error in script", quoted(path.Str()),
    "specified with --run:\n") + format_error_info(err);
}

static PaintMap default_palette(){
  PaintMap palette;
  palette.Append(Paint(Color(0, 0, 0, 0)));
  palette.Append(Paint(Color(255,0,255)));
  palette.Append(Paint(Color(0, 0, 0)));
  palette.Append(Paint(Color(255,255,255)));
  palette.Append(Paint(Color(70, 70, 70)));
  palette.Append(Paint(Color(220,220,220)));
  palette.Append(Paint(Color(120, 120, 120)));
  palette.Append(Paint(Color(180,180,180)));
  palette.Append(Paint(Color(153, 0, 48)));
  palette.Append(Paint(Color(156, 90, 60)));
  palette.Append(Paint(Color(237, 28, 36)));
  palette.Append(Paint(Color(255, 163, 177)));
  palette.Append(Paint(Color(255, 126, 0)));
  palette.Append(Paint(Color(229, 170, 122)));
  palette.Append(Paint(Color(255, 242, 0)));
  palette.Append(Paint(Color(245, 228, 156)));
  palette.Append(Paint(Color(255, 194, 14)));
  palette.Append(Paint(Color(255, 249, 189)));
  palette.Append(Paint(Color(168, 230, 29)));
  palette.Append(Paint(Color(211, 249, 188)));
  palette.Append(Paint(Color(34, 177, 76)));
  palette.Append(Paint(Color(157, 187, 97)));
  palette.Append(Paint(Color(0, 183, 239)));
  palette.Append(Paint(Color(153, 217, 234)));
  palette.Append(Paint(Color(77, 109, 243)));
  palette.Append(Paint(Color(112, 154, 209)));
  palette.Append(Paint(Color(47, 54, 153)));
  palette.Append(Paint(Color(84, 109, 142)));
  palette.Append(Paint(Color(111, 49, 152)));
  palette.Append(Paint(Color(181, 165, 213)));
  return palette;
}

class Application : public wxApp{
public:
  Application()
  {}

  int FilterEvent(wxEvent& untypedEvent) override{
    // Filter for key events so that bound keys are relayed to Python
    // regardless of what control has focus. This prevents controls like
    // tool buttons, the aui-toolbar etc. from swallowing keypresses
    // when they have focus.

    wxEventType eventType = untypedEvent.GetEventType();
    if (eventType != wxEVT_KEY_DOWN && eventType != wxEVT_KEY_UP){
      return Event_Skip;
    }
    const wxKeyEvent& event = (wxKeyEvent&)untypedEvent;
    const int keyCode = event.GetKeyCode();

    if (key::modifier(keyCode)){
      m_faintWindow->ModifierKeyChange();

      // Modifier keys (like Ctrl, Shift) can not be bound separately -
      // stop special handling here.
      return Event_Skip;
    }
    if (eventType == wxEVT_KEY_UP){
      // Key up is only relevant for modifier refresh.
      return Event_Skip;
    }

    KeyPress key(key_modifiers(event), Key(keyCode));
    const bool boundGlobal = m_pythonContext->BoundGlobal(key);
    const bool bound = m_pythonContext->Bound(key);
    if (!bound && !boundGlobal){
      // Allow normal key handling for unbound keys
      return Event_Skip;
    }

    if (!m_faintWindow->Focused() && !boundGlobal){
      // Ignore non-global Python-binds if a window or dialog is shown
      // above the mainframe
      return Event_Skip;
    }

    // Note: It appears that FilterEvent does not happen while a menu is
    // open, so no check to avoid this is performed.

    // Non-global binds must check if text-entry is active
    if (!boundGlobal){
      EntryMode entryMode = m_faintWindow->GetTextEntryMode();
      if (entryMode == EntryMode::ALPHA_NUMERIC &&
        affects_alphanumeric_entry(key))
      {
        return Event_Skip;
      }
      else if (entryMode == EntryMode::NUMERIC && affects_numeric_entry(key)){
        return Event_Skip;
      }
    }

    // Run the key bind, and swallow the key-press.
    python_key_press(key);
    return Event_Processed;
  }

  AppContext& GetAppContext() const{
    return *m_appContext;
  }

  PythonContext& GetPythonContext() const{
    return *m_pythonContext;
  }

  const ArtContainer& GetArtContainer() const{
    return m_art;
  }

  bool OnCmdLineParsed(wxCmdLineParser& parser) override{
    m_cmd.silentMode = parser.Found("s");
    m_cmd.forceNew = parser.Found("i");
    m_cmd.preventServer = parser.Found("ii");
    m_cmd.port = get_string(parser, "port", get_default_faint_port());
    m_cmd.arg = get_string(parser, "arg", "");
    utf8_string scriptPath = get_string(parser, "run");
    m_cmd.scriptPath = make_absolute_file_path(scriptPath);

    if (!valid_port(m_cmd.port.str())){
      console_message("Error: Invalid port specified " +
        bracketed(m_cmd.port.str()));
      return false;
    }

    if (m_cmd.silentMode && m_cmd.scriptPath.NotSet()){
      console_message("Error: --silent requires a script specified with "
        "--run <scriptname>");
      return false;
    }

    for (size_t i = 0; i!= parser.GetParamCount(); i++){
      const wxString param(parser.GetParam(i));
      wxFileName absPath(absoluted(wxFileName(param)));
      if (absPath.IsDir()){
        console_message(wxString("Error: Folder path specified on command "
            "line - image path expected (") + param + ").");
        return false;
      }
      m_cmd.files.push_back(FilePath::FromAbsoluteWx(absPath));
    }
    return true;
  }

  bool OnExceptionInMainLoop() override{
    if (m_crashFile.IsSet()){
      write_exception_log(m_crashFile.Get());
    }
    std::abort();
  }

  int OnExit() override{
    m_appContext.reset(nullptr);
    m_faintInstance.reset(nullptr);
    m_faintWindow.reset(nullptr);
    m_helpFrame.reset(nullptr);
    m_interpreterFrame.reset(nullptr);
    m_pythonContext.reset(nullptr);
    return wxApp::OnExit();
  }

  bool OnInit() override{
    assert(CallOrder(0));

    // Perform default init for command-line etc.
    if (!wxApp::OnInit()){
      return false;
    }

    // Store the path to the crash-log file to require minimum effort
    // to write it on unhandled exception.
    m_crashFile = get_crash_file();

    m_faintInstance = create_faint_instance(m_cmd.files,
      allow_server(!m_cmd.preventServer), force_start(m_cmd.forceNew),
      m_cmd.port.str());

    if (!m_faintInstance->AllowStart()){
      return false;
    }

    wxInitAllImageHandlers();
    m_art.SetRoot(get_data_dir().SubDir("graphics"));
    load_faint_resources(m_art);

    // Create frames and restore their states from the last run
    m_interpreterFrame = std::make_unique<InterpreterFrame>();
    m_interpreterFrame->SetIcons(get_icon(m_art, Icon::FAINT_PYTHON16),
      get_icon(m_art, Icon::FAINT_PYTHON32));

    m_helpFrame = std::make_unique<HelpFrame>(get_help_dir(), m_art);
    m_helpFrame->SetIcons(get_icon(m_art, Icon::HELP16),
      get_icon(m_art, Icon::HELP32));

    m_faintWindow = std::make_unique<FaintWindow>(m_art,
      default_palette(),
      m_helpFrame.get(),
      m_interpreterFrame.get(),
      m_cmd.silentMode);

    m_faintWindow->SetIcons(get_icon(m_art, Icon::FAINT16),
      get_icon(m_art, Icon::FAINT32));
    m_appContext.reset(&(m_faintWindow->GetAppContext()));

    m_pythonContext.reset(&(m_faintWindow->GetPythonContext()));

    bool ok = init_python(m_cmd.arg);
    if (!ok){
      show_error(null_parent(), Title("Faint Internal Error"),
        "Faint crashed!\n\n...while running envsetup.py");
      // Fixme: Previously deleted m_faintWindow here, when it was a wxFrame.
      // Must the frame be deleted on error if before SetTopWindow?
      return false;
    }

    if (!m_cmd.silentMode){
      m_faintWindow->Show();
    }

    m_faintWindow->Initialize();

    bool configOk = run_python_user_config(*m_pythonContext);
    if (!configOk){
      // Show the console where some error info should have been printed.
      m_appContext->ShowPythonConsole();
      if (m_cmd.scriptPath.IsSet()){
        m_pythonContext->IntFaintPrint(space_sep("Script",
          quoted(m_cmd.scriptPath.Get().Str()),
            "ignored due to configuration file error.\n"));
        m_cmd.scriptPath.Clear();
      }
    }
    m_interpreterFrame->AddNames(list_ifaint_names());
    if (!m_cmd.files.empty()){
      m_faintWindow->Open(m_cmd.files);
    }

    SetTopWindow(&m_faintWindow->GetRawFrame());
    return true;
  }

  void OnInitCmdLine(wxCmdLineParser& parser) override{
    parser.SetDesc(g_cmdLineDesc);
  }

  void RunScript(const FilePath& scriptPath){
    if (!exists(scriptPath)){
      if (m_cmd.silentMode){
        console_message(to_wx(space_sep(utf8_string(
          "Python file specified with --run not found:"),
          scriptPath.Str())));
      }
      else {
        show_error(null_parent(), Title("Script not found"),
          to_wx(endline_sep(utf8_string("Python file specified with --run not found:"), scriptPath.Str())));
      }
    }
    else {
      run_python_file(scriptPath).Visit(
        [&](const FaintPyExc& err){
          const utf8_string errStr(format_run_script_error(scriptPath, err));
          if (m_cmd.silentMode){
            console_message(to_wx(errStr));
          }
          else{
            m_pythonContext->IntFaintPrint(errStr);
            m_appContext->ShowPythonConsole();
          }
        });
    }
  }

  int OnRun() override{
    assert(CallOrder(1));
    m_cmd.scriptPath.Visit(
      [&](const FilePath& path){
        RunScript(path);
      });

    if (m_cmd.silentMode){
      return 0; // Exit
    }

    m_pythonContext->NewPrompt();
    return wxApp::OnRun();
  }

private:
  bool CallOrder(int expected){
    return m_callNum++ == expected;
  }

  std::unique_ptr<AppContext> m_appContext;
  ArtContainer m_art;
  int m_callNum = 0;
  CommandLine m_cmd;
  std::unique_ptr<FaintInstance> m_faintInstance;
  std::unique_ptr<FaintWindow> m_faintWindow;
  std::unique_ptr<HelpFrame> m_helpFrame;
  std::unique_ptr<InterpreterFrame> m_interpreterFrame;
  std::unique_ptr<PythonContext> m_pythonContext;
  Optional<FilePath> m_crashFile;
};

} // namespace

IMPLEMENT_APP(faint::Application)

namespace faint{

AppContext& get_app_context(){
  return wxGetApp().GetAppContext();
}

PythonContext& get_python_context(){
  return wxGetApp().GetPythonContext();
}

const ArtContainer& get_art_container(){
  return wxGetApp().GetArtContainer();
}

} // namespace
