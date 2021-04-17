// -*- coding: us-ascii-unix -*-
// Copyright 2020 Lukas Kemmer
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

// Error strings for file-format read functions.

#include "text/utf8-string.hh"

namespace faint{ class FilePath; }

namespace faint{

utf8_string error_open_file_read(const FilePath&);

} // namespace
