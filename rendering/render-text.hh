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
#include "text/text-buffer.hh" // Fixme: Remove, pass CaretRange
#include "util/settings.hh"

namespace faint{

// Fixme: Move elsewhere, add unit test.
LineSegment compute_caret(const TextInfo&,
  const TextBuffer&,
  const Tri&,
  const text_lines_t&,
  const Settings&);

void render_text(FaintDC&,
  const text_lines_t&,
  const CaretRange&,
  const Tri&,
  bool currentlyEdited,
  TextInfo&,
  const Settings&);

void render_text_mask(
  FaintDC&,
  const TextBuffer&,
  const Tri&,
  TextInfo&,
  const Settings&);

} // namespace

#endif
