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

#include "app/get-app-context.hh"
#include "app/save.hh"
#include "formats/format.hh"
#include "text/formatting.hh"
#include "util-wx/file-format-util.hh"
#include "util-wx/file-path.hh"

namespace faint{

SaveResult save(Canvas& canvas, const FilePath& path){
  Format* format = get_save_format(get_app_context().GetFormats(),
    path.Extension());
  if (format == nullptr){
    return SaveResult::SaveFailed(str_no_matching_format(path.Extension()));
  }
  return format->Save(path, canvas);
}

utf8_string str_no_matching_format(const FileExtension& ext){
  return space_sep(
    utf8_string("No available format for saving with extension"),
    ext.Str() + utf8_string("."));
}

} // namespace
