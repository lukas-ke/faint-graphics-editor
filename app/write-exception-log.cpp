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

#include <string>
#include "wx/filename.h"
#include "wx/stdpaths.h"
#include "app/write-exception-log.hh"
#include "bitmap/bitmap-exception.hh"
#include "util-wx/stream.hh"

namespace faint{

static std::string get_exception_string(){
  try{
    throw;
  }
  catch (const BitmapException& e){
    return std::string("BitmapException: ") + e.what();
  }
  catch (const std::exception& e){
    return std::string("std::exception: ") + e.what();
  }
  catch (...){
    return "unknown exception";
  }
}

Optional<FilePath> get_crash_file(){
  auto tempDir = wxStandardPaths::Get().GetTempDir();
  if (tempDir.empty()){
    return no_option();
  }
  else{
    return {FilePath::FromAbsoluteWx(wxFileName(tempDir, "faint-crash-info.txt"))};
  }
}

void write_exception_log(const FilePath& path){
  auto s = get_exception_string();
  BinaryWriter w(path);
  if (w.good()){
    auto msg = get_exception_string();
    w.write(msg.c_str(), msg.size());
  }
}

} // namespace
