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
#include <functional>
#include "gui/command-dialog.hh"

namespace faint{

class Art;
class DialogContext;
class Canvas;
class Settings;
enum class ToolId;

void show_alpha_dialog(DialogContext&);

Optional<BitmapCommand*> show_brightness_contrast_dialog(wxWindow&,
  DialogContext&,
  DialogFeedback&);

Optional<BitmapCommand*> show_color_balance_dialog(wxWindow&,
  DialogContext&,
  DialogFeedback&);

void show_faint_about_dialog(wxWindow* parent, DialogContext&);

Optional<BitmapCommand*> show_pinch_whirl_dialog(wxWindow&,
  DialogContext&,
  DialogFeedback&);

Optional<Command*> show_rotate_dialog_old(wxWindow&,
  const Canvas&, // Fixme: Why?
  const Art&,
  const std::function<Paint()>& bgColor,
  const std::function<void()>& selectLevelTool,
  DialogContext&);

dialog_func bind_show_rotate_dialog_old(const Art&,
  const std::function<Paint()>& bgColor,
  const std::function<void(ToolId)>& selectLevelTool,
  DialogContext&);

void show_threshold_dialog(DialogContext&, const Settings&);

Optional<BitmapCommand*> show_pixelize_dialog(wxWindow&,
  DialogContext&,
  DialogFeedback&);

Optional<BitmapCommand*> show_sharpness_dialog(wxWindow&,
  DialogContext&,
  DialogFeedback&);

} // namespace

#endif
