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

#include <wx/settings.h>
#include "bitmap/draw.hh"
#include "geo/geo-func.hh"
#include "geo/int-rect.hh"
#include "geo/line.hh"
#include "util-wx/system-colors.hh"
#include "util-wx/convert-wx.hh"

namespace faint{

static Color get(wxSystemColour c){
  return to_faint(wxSystemSettings::GetColour(c));
}

Color color_system_scrollbar(){
  return get(wxSYS_COLOUR_SCROLLBAR);
}

Color color_desktop(){
  return get(wxSYS_COLOUR_DESKTOP);
}

Color color_active_caption(){
  return get(wxSYS_COLOUR_ACTIVECAPTION);
}

Color color_inactive_caption(){
  return get(wxSYS_COLOUR_INACTIVECAPTION);
}

Color color_menu(){
  return get(wxSYS_COLOUR_MENU);
}

Color color_window(){
  return get(wxSYS_COLOUR_WINDOW);
}

Color color_windowframe(){
  return get(wxSYS_COLOUR_WINDOWFRAME);
}

Color color_menutext(){
  return get(wxSYS_COLOUR_MENUTEXT);
}

Color color_windowtext(){
  return get(wxSYS_COLOUR_WINDOWTEXT);
}

Color color_captiontext(){
  return get(wxSYS_COLOUR_CAPTIONTEXT);
}

Color color_active_border(){
  return get(wxSYS_COLOUR_ACTIVEBORDER);
}

Color color_inactive_border(){
  return get(wxSYS_COLOUR_INACTIVEBORDER);
}

Color color_app_workspace(){
  return get(wxSYS_COLOUR_APPWORKSPACE);
}

Color color_highlight(){
  return get(wxSYS_COLOUR_HIGHLIGHT);
}

Color color_highlighttext(){
  return get(wxSYS_COLOUR_HIGHLIGHTTEXT);
}

Color color_button_face(){
  return get(wxSYS_COLOUR_BTNFACE);
}

Color color_system_face_3d(){
  return get(wxSYS_COLOUR_3DFACE);
}

Color color_button_shadow(){
  return get(wxSYS_COLOUR_BTNSHADOW);
}

Color color_graytext(){
  return get(wxSYS_COLOUR_GRAYTEXT);
}

Color color_button_text(){
  return get(wxSYS_COLOUR_BTNTEXT);
}

Color color_inactive_caption_text(){
  return get(wxSYS_COLOUR_INACTIVECAPTIONTEXT);
}

Color color_button_highlight(){
  return get(wxSYS_COLOUR_BTNHIGHLIGHT);
}

Color color_dark_shadow_3d(){
  return get(wxSYS_COLOUR_3DDKSHADOW);
}

Color color_system_light_3d(){
  return get(wxSYS_COLOUR_3DLIGHT);
}

Color color_info_text(){
  return get(wxSYS_COLOUR_INFOTEXT);
}

Color color_info_background(){
  return get(wxSYS_COLOUR_INFOBK);
}

Color color_listbox(){
  return get(wxSYS_COLOUR_LISTBOX);
}

Color color_hotlight(){
  return get(wxSYS_COLOUR_HOTLIGHT);
}

Color color_gradient_active_caption(){
  return get(wxSYS_COLOUR_GRADIENTACTIVECAPTION);
}

Color color_gradient_inactive_caption(){
  return get(wxSYS_COLOUR_GRADIENTINACTIVECAPTION);
}

Color color_menu_highlight(){
  return get(wxSYS_COLOUR_MENUHILIGHT);
}

Color color_menubar(){
  return get(wxSYS_COLOUR_MENUBAR);
}

Color color_listbox_text(){
  return get(wxSYS_COLOUR_LISTBOXTEXT);
}

Color color_listbox_highlight_text(){
  return get(wxSYS_COLOUR_LISTBOXHIGHLIGHTTEXT);
}

void draw_sunken_ui_border(Bitmap& bmp, const IntRect& r){
  const auto light = solid_1px(color_button_highlight());
  const auto dark = solid_1px(color_button_shadow());
  draw_hline(bmp, r.Top(), left_right(r), dark);
  draw_hline(bmp, r.Bottom(), left_right(r), light);
  draw_vline(bmp, r.Left(), top_bottom(r), dark);
  draw_vline(bmp, r.Right(), top_bottom(r), light);
}

void draw_raised_ui_border(Bitmap& bmp, const IntRect& r){
  const auto border = solid_1px(color_active_border());
  draw_hline(bmp, r.Top(), left_right(r), border);
  draw_hline(bmp, r.Bottom(), left_right(r), border);
  draw_vline(bmp, r.Left(), top_bottom(r), border);
  draw_vline(bmp, r.Right(), top_bottom(r), border);
}

} // namespace
