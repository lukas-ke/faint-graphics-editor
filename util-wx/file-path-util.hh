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

#ifndef FAINT_FILE_PATH_UTIL_HH
#define FAINT_FILE_PATH_UTIL_HH
#include "util-wx/file-path.hh"

namespace faint{

// Returns the root for data sub-folders
DirPath get_data_dir();

// Returns the folder containing html-help files
DirPath get_help_dir();

// Returns the users home dir (as determined by wxWidgets)
DirPath get_home_dir();

// Returns the folder containing palette files
DirPath get_palette_dir();

FilePath get_faint_exe_path();

// Returns the path to the user's configuration Python script
// (regardless of if it exists or not)
FilePath get_user_config_file_path();

} // namespace

#endif
