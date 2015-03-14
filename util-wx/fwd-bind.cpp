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

#include "wx/wx.h"
#include "fwd-bind.hh"
#include "geo/int-point.hh"
#include "util-wx/bind-event.hh"
#include "util-wx/convert-wx.hh"

namespace faint{ namespace events{

void on_button(wxButton* b, const void_func& f){
  bind(b, wxEVT_BUTTON, f);
}

void on_close_window(window_t w, const void_func& f){
  bind(w.w, wxEVT_CLOSE_WINDOW, f);
}

void on_idle(window_t w, const void_func& f){
  bind(w.w, wxEVT_IDLE, f);
}

void on_kill_focus(window_t w, const void_func& f){
  bind_skip(w.w, wxEVT_KILL_FOCUS, f);
}

void on_menu_choice_all(window_t w, const int_func& f){
  bind_menu_all(w.w, f);
}

void on_mouse_enter_window(window_t w, const void_func& f){
  bind(w.w, wxEVT_ENTER_WINDOW, f);
}

void on_mouse_leave_window(window_t w, const void_func& f){
  bind(w.w, wxEVT_LEAVE_WINDOW, f);
}

static void bind_mouse_event(window_t w,
  const wxEventTypeTag<wxMouseEvent>& tag,
  const int_point_func& f)
{
  bind_fwd(w.w, tag,
    [f](wxMouseEvent& e){
      f(to_faint(e.GetPosition()));
      e.Skip();
    });
}

void on_mouse_left_double_click(window_t w, const int_point_func& f){
  bind_mouse_event(w, wxEVT_LEFT_DCLICK, f);
}

void on_mouse_left_down(window_t w, const int_point_func& f){
  bind_mouse_event(w, wxEVT_LEFT_DOWN, f);
}

void on_mouse_left_up(window_t w, const int_point_func& f){
  bind_mouse_event(w, wxEVT_LEFT_UP, f);
}

void on_mouse_motion(window_t w, const int_point_func& f){
  bind_mouse_event(w.w, wxEVT_MOTION, f);
}

void on_mouse_right_down(window_t w, const int_point_func& f){
  bind_mouse_event(w.w, wxEVT_RIGHT_DOWN, f);
}

void on_mouse_right_double_click(window_t w, const int_point_func& f){
  bind_mouse_event(w.w, wxEVT_RIGHT_DCLICK, f);
}

void on_mouse_right_up(window_t w, const int_point_func& f){
  bind_mouse_event(w.w, wxEVT_RIGHT_UP, f);
}

void on_paint(window_t w, const void_func& f){
  bind(w.w, wxEVT_PAINT, f);
}

void on_resize(window_t w, const void_func& f){
  bind_skip(w.w, wxEVT_SIZE, f);
}

void on_set_focus(window_t w, const void_func& f){
  bind_skip(w.w, wxEVT_SET_FOCUS, f);
}

void no_op_erase_background(window_t w){
  bind(w.w, wxEVT_ERASE_BACKGROUND, [](){});
}

}} // namespace
