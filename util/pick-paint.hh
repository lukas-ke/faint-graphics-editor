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

#ifndef FAINT_PICK_PAINT_HH
#define FAINT_PICK_PAINT_HH
#include <functional>
#include "bitmap/paint.hh"
#include "text/utf8-string.hh"
#include "util/optional.hh"

namespace faint{

// Type of function optionally returning a Paint. Typically used to
// provide a color-picker dialog (e.g. a color dialog) to controls,
// which the control can show when necessary.
using pick_paint_f = std::function<Optional<Paint>(
  const utf8_string& title,
  const Paint& initial,
  const Color& secondary)>;

} // namespace

#endif
