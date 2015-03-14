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

#ifndef FAINT_FWD_BIND_HH
#define FAINT_FWD_BIND_HH
#include <functional>
#include "util-wx/window-types-wx.hh"

class wxButton;
class wxTextCtrl;
namespace faint{ class IntPoint; }

namespace faint{ namespace events{

using void_func = std::function<void()>;
using int_point_func = std::function<void(const IntPoint&)>;
using int_func = std::function<void(int)>;

// Helpers for binding wxWidgets events to functions

void on_button(wxButton*, const void_func&);
void on_close_window(window_t, const void_func&);
void on_idle(window_t, const void_func&);
void on_kill_focus(window_t, const void_func&);
void on_menu_choice_all(window_t, const int_func&);
void on_mouse_enter_window(window_t, const void_func&);
void on_mouse_leave_window(window_t, const void_func&);
void on_mouse_left_double_click(window_t, const int_point_func&);
void on_mouse_left_down(window_t, const int_point_func&);
void on_mouse_right_double_click(window_t, const int_point_func&);
void on_mouse_right_down(window_t, const int_point_func&);
void on_mouse_right_up(window_t, const int_point_func&);
void on_mouse_left_up(window_t, const int_point_func&);
void on_mouse_motion(window_t, const int_point_func&);
void on_paint(window_t, const void_func&);
void on_resize(window_t, const void_func&);
void on_set_focus(window_t, const void_func&);
void no_op_erase_background(window_t);

}} // namespace

#endif
