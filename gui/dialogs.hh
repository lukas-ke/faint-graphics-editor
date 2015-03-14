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

#ifndef FAINT_DIALOGS_HH
#define FAINT_DIALOGS_HH
#include "gui/command-dialog.hh"

namespace faint{

class ArtContainer;
class DialogContext;
class Canvas;
class Settings;

void show_alpha_dialog(DialogContext&);

Optional<BitmapCommand*> show_brightness_contrast_dialog(wxWindow&,
  DialogFeedback&);

Optional<BitmapCommand*> show_color_balance_dialog(wxWindow&, DialogFeedback&);

void show_faint_about_dialog(wxWindow* parent, DialogContext&);

Optional<BitmapCommand*> show_pinch_whirl_dialog(wxWindow&, DialogFeedback&);

Optional<Command*> show_rotate_dialog(wxWindow&,
  const Canvas&, // Fixme: Why?
  const ArtContainer&,
  DialogContext&);

dialog_func bind_show_rotate_dialog(const ArtContainer&, DialogContext&);

void show_threshold_dialog(DialogContext&, const Settings&);

Optional<BitmapCommand*> show_pixelize_dialog(wxWindow&, DialogFeedback&);

Optional<BitmapCommand*> show_sharpness_dialog(wxWindow&, DialogFeedback&);

} // namespace

#endif
