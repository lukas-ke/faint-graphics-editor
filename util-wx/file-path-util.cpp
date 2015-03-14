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

#include "text/utf8-string.hh"
#include "util-wx/file-path-util.hh"

namespace faint{

// Some of the functions are defined in util-wx.cpp

FilePath get_user_config_file_path(){
  return get_home_dir().File(".faint.py");
}

DirPath get_palette_dir(){
  return get_data_dir().SubDir("palettes");
}

DirPath get_help_dir(){
  return get_data_dir().SubDir("help");
}

}
