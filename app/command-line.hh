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

#ifndef FAINT_COMMAND_LINE_HH
#define FAINT_COMMAND_LINE_HH
#include "wx/cmdline.h"
#include "text/utf8-string.hh"
#include "util/optional.hh"
#include "util/or-error.hh"
#include "util-wx/file-path.hh"

namespace faint{

class CommandLine{
public:
  CommandLine();
  bool forceNew; // single instance
  bool preventServer; // single instance
  bool silentMode;
  bool script;
  Optional<FilePath> scriptPath;
  utf8_string port;
  FileList files;
  utf8_string arg; // Script argument
  bool usePenTablet;
};

void init_command_line_parser(wxCmdLineParser&);
OrError<CommandLine> get_parsed_command_line(wxCmdLineParser&);

} // namespace

#endif
