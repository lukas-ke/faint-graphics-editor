// -*- coding: us-ascii-unix -*-
// Copyright 2013 Lukas Kemmer
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

#ifndef FAINT_FILE_FORMAT_UTIL_HH
#define FAINT_FILE_FORMAT_UTIL_HH
#include <string>
#include <vector>
#include "formats/format.hh"

namespace faint{

using Formats = std::vector<Format*>;

Formats built_in_file_formats();

Formats loading_file_formats(const Formats&);

Formats saving_file_formats(const Formats&);

utf8_string file_dialog_filter(const std::vector<Format*>&);

utf8_string combined_file_dialog_filter(const utf8_string& description,
  const Formats&);

int get_file_format_index(const Formats&, const FileExtension&);

// Fixme: Return Optional<Format&>
Format* get_load_format(const Formats&, const FileExtension&);

Format* get_save_format(const Formats&, const FileExtension&);

Format* get_save_format(const Formats&, const FileExtension&,
  int filterIndex);

bool has_save_format(const Formats&, const FileExtension&);

} // namespace

#endif
