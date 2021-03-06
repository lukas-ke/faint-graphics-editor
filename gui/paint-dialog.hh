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

#ifndef FAINT_PAINT_DIALOG_HH
#define FAINT_PAINT_DIALOG_HH
#include "bitmap/paint.hh"
#include "gui/dialog-context.hh"
#include "util/accessor.hh"
#include "util/optional.hh"

class wxWindow;

namespace faint{

class Bitmap;
class StatusInterface;
class utf8_string;

// Shows a dialog for selecting an RGBA-color.
Optional<Color> show_color_only_dialog(wxWindow* parent,
  const utf8_string& title,
  const Color& initial,
  DialogContext&);

// Shows a dialog for selecting a Paint.
//
// The 'initial'-Paint parameter determines what page will be selected
// first (color, gradient or pattern), and what that values
// the page will be initialized with.
// Typically, the current selection should be used for initial.
Optional<Paint> show_paint_dialog(wxWindow* parent,
  const utf8_string& title,
  const Paint& initial,
  const Color& secondary,
  const Getter<Bitmap>&,
  StatusInterface&,
  DialogContext&);

} // namespace

#endif
