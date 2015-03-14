// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
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

#ifndef FAINT_FILE_CUR_HH
#define FAINT_FILE_CUR_HH
#include "bitmap/bitmap-fwd.hh"
#include "formats/save-result.hh"
#include "util/hot-spot.hh"
#include "util/or-error.hh"
#include "util-wx/file-path.hh"

namespace faint{

using cur_vec = std::vector<std::pair<Bitmap, HotSpot>>;

OrError<cur_vec> read_cur(const FilePath&);

SaveResult write_cur(const FilePath&, const cur_vec&);

} // namespace

#endif
