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

#ifndef FAINT_RENDER_TEXT_HH
#define FAINT_RENDER_TEXT_HH
#include "geo/line.hh" // LineSegment
#include "rendering/faint-dc.hh"
#include "text/text-line.hh" // TextInfo
#include "text/caret.hh"
#include "util/settings.hh"

namespace faint{

// Returns a line for the graphical caret position
LineSegment compute_caret(const TextInfo&,
  Caret, // caret in the un-split text
  const Tri&,
  const text_lines_t&, // the text broken into lines
  const Settings&);

void render_text(FaintDC&,
  const text_lines_t&,
  const CaretRange&,
  const Tri&,
  bool currentlyEdited,
  TextInfo&,
  const Settings&);

void render_text_mask(FaintDC&,
  const text_lines_t&,
  const Tri&,
  TextInfo&,
  const Settings&,
  const Paint& withinBounds,
  const Paint& withinLine);

} // namespace

#endif
