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

#ifndef FAINT_RESIZE_CMD_HH
#define FAINT_RESIZE_CMD_HH
#include "geo/geo-fwd.hh"
#include "text/utf8-string.hh"
#include "util/distinct.hh"

namespace faint{

class Command;

using AltIntRect = Alternative<IntRect>;
using AltPaint = Alternative<Paint>;

// Command for resizing the image to the specified rectangle, using
// the Paint as the fill if growing.
Command* resize_command(const IntRect&, const Paint&,
  const utf8_string& name="Resize Image");

// Resize with an alternate IntRect used for DWIM-resize.
Command* resize_command(const IntRect&, const AltIntRect&, const Paint&,
  const utf8_string& name="Resize Image");

// Resize with an alternate Paint for DWIM-resize.
Command* resize_command(const IntRect&, const Paint&, const AltPaint&,
  const utf8_string& name="Resize Image");

} // namespace

#endif
