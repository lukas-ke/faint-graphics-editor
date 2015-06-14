// -*- coding: us-ascii-unix -*-
// Copyright 2015 Lukas Kemmer
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

#include <cctype>
#include <sstream>
#include "app/command-line.hh"
#include "text/formatting.hh"
#include "util-wx/convert-wx.hh"
#include "wx/filename.h"

namespace faint{

static utf8_string get_default_faint_port(){
  return utf8_string("3793");
}

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

CommandLine::CommandLine() :
  forceNew(false),
  preventServer(false),
  silentMode(false),
  script(false),
  port(get_default_faint_port()),
  usePenTablet(true)
{}

const char* CMD_HELP_SHORT = "h";
const char* CMD_HELP_LONG = "help";

const char* CMD_SILENT_SHORT = "s";
const char* CMD_SILENT_LONG = "silent";

const char* CMD_NO_PEN_TABLET = "no-tablet";

const char* CMD_NEW_INSTANCE_SHORT = "i";
const char* CMD_NEW_INSTANCE_LONG = "new-instance";

const char* CMD_NO_SERVER = "no-server";

const char* CMD_SERVER_PORT = "port";

const char* CMD_RUN_SCRIPT = "run";

const char* CMD_SCRIPT_ARG = "arg";

static const wxCmdLineEntryDesc g_cmdLineDesc[] = {
  {wxCMD_LINE_SWITCH, CMD_HELP_SHORT, CMD_HELP_LONG,
    "Displays help on the command line parameters",
   wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP},

  {wxCMD_LINE_SWITCH, CMD_SILENT_SHORT, CMD_SILENT_LONG,
   "Disables the GUI. Requires specifying a script with with --run.",
   wxCMD_LINE_VAL_STRING,
   wxCMD_LINE_PARAM_OPTIONAL},

  {wxCMD_LINE_SWITCH, "", CMD_NO_PEN_TABLET,
   "Disable initialization of pen tablet.",
   wxCMD_LINE_VAL_STRING,
   wxCMD_LINE_PARAM_OPTIONAL},

  {wxCMD_LINE_PARAM, "", "",
   "Image files",
   wxCMD_LINE_VAL_STRING,
   wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE},

  {wxCMD_LINE_SWITCH, CMD_NEW_INSTANCE_SHORT, CMD_NEW_INSTANCE_LONG,
   "Force new instance",
   wxCMD_LINE_VAL_STRING,
   wxCMD_LINE_PARAM_OPTIONAL},

  {wxCMD_LINE_SWITCH, "", CMD_NO_SERVER,
   "Prevent this instance from becoming a main app",
   wxCMD_LINE_VAL_STRING,
   wxCMD_LINE_PARAM_OPTIONAL},

  {wxCMD_LINE_OPTION, "", CMD_SERVER_PORT,
   "Specify port used for IPC.",
   wxCMD_LINE_VAL_STRING,
   wxCMD_LINE_PARAM_OPTIONAL},

  {wxCMD_LINE_OPTION, "", CMD_RUN_SCRIPT,
   "Run a Python script file after loading images",
   wxCMD_LINE_VAL_STRING,
   wxCMD_LINE_PARAM_OPTIONAL},

  {wxCMD_LINE_OPTION, "", CMD_SCRIPT_ARG,
   "Custom argument stored in ifaint.cmd_arg.", // Fixme: Duplication
   wxCMD_LINE_VAL_STRING, wxCMD_LINE_PARAM_OPTIONAL},

  { // Sentinel
    wxCMD_LINE_NONE, "", "", "",
    wxCMD_LINE_VAL_NONE,
    wxCMD_LINE_PARAM_OPTIONAL}
};

void init_command_line_parser(wxCmdLineParser& p){
  p.SetDesc(g_cmdLineDesc);
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

OrError<CommandLine> get_parsed_command_line(wxCmdLineParser& p){
  CommandLine cmd;
  cmd.silentMode = p.Found(CMD_SILENT_LONG);
  cmd.usePenTablet = !cmd.silentMode && !p.Found(CMD_NO_PEN_TABLET);
  cmd.forceNew = p.Found(CMD_NEW_INSTANCE_LONG);
  cmd.preventServer = p.Found(CMD_NO_SERVER);
  cmd.port = get_string(p, CMD_SERVER_PORT, get_default_faint_port());
  cmd.arg = get_string(p, CMD_SCRIPT_ARG, "");

  utf8_string scriptPath = get_string(p, CMD_RUN_SCRIPT);
  cmd.scriptPath = make_absolute_file_path(scriptPath);

  if (!valid_port(cmd.port.str())){
    return {space_sep("Error: Invalid port specified",
      bracketed(cmd.port.str().c_str()))};
  }

  if (cmd.silentMode && cmd.scriptPath.NotSet()){
    return {space_sep("Error:", no_sep("--", CMD_SILENT_LONG),
      "requires a script specified with", no_sep("--", CMD_RUN_SCRIPT), "<scriptname>")};
  }

  for (size_t i = 0; i!= p.GetParamCount(); i++){
    const wxString param(p.GetParam(i));
    wxFileName absPath(absoluted(wxFileName(param)));
    if (absPath.IsDir()){
      return {space_sep(
        "Error: Folder path specified on command line - image path expected",
        bracketed(to_faint(param)))};
    }
    cmd.files.push_back(FilePath::FromAbsoluteWx(absPath));
  }

  return {cmd};
}

} // namespace
