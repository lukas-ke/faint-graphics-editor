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

#ifndef FAINT_SYSTEM_COLORS_HH
#define FAINT_SYSTEM_COLORS_HH
#include "bitmap/color.hh"

class Bitmap;
class IntRect;

namespace faint{

Color color_active_border();
Color color_active_caption();
Color color_app_workspace();
Color color_button_face();
Color color_button_highlight();
Color color_button_shadow();
Color color_button_text();
Color color_captiontext();
Color color_dark_shadow_3d();
Color color_desktop();
Color color_gradient_active_caption();
Color color_gradient_inactive_caption();
Color color_graytext();
Color color_highlight();
Color color_highlighttext();
Color color_hotlight();
Color color_inactive_border();
Color color_inactive_caption();
Color color_inactive_caption_text();
Color color_info_background();
Color color_info_text();
Color color_listbox();
Color color_listbox_highlight_text();
Color color_listbox_text();
Color color_menu();
Color color_menu_highlight();
Color color_menubar();
Color color_menutext();
Color color_system_face_3d();
Color color_system_light_3d();
Color color_system_scrollbar();
Color color_window();
Color color_windowframe();
Color color_windowtext();

void draw_sunken_ui_border(Bitmap&, const IntRect&);
void draw_raised_ui_border(Bitmap&, const IntRect&);

} // namespace

#endif
