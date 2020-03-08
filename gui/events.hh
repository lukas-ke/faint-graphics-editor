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

#ifndef FAINT_EVENTS_HH
#define FAINT_EVENTS_HH
#include "wx/event.h"
#include "bitmap/color.hh"
#include "bitmap/paint.hh"
#include "tools/tool-id.hh"
#include "util-wx/file-path.hh"
#include "util-wx/fwd-bind.hh"

namespace faint{

using CommandEventTag = const wxEventTypeTag<wxCommandEvent>;

// Zoom command events declarations
extern const wxEventType FAINT_ZoomIn;
extern CommandEventTag EVT_FAINT_ZoomIn;

extern const wxEventType FAINT_ZoomInAll;
extern CommandEventTag EVT_FAINT_ZoomInAll;

extern const wxEventType FAINT_ZoomOut;
extern CommandEventTag EVT_FAINT_ZoomOut;

extern const wxEventType FAINT_ZoomOutAll;
extern CommandEventTag EVT_FAINT_ZoomOutAll;

extern const wxEventType FAINT_ZoomActualSize;
extern CommandEventTag EVT_FAINT_ZoomActualSize;

extern const wxEventType FAINT_ZoomActualSizeAll;
extern CommandEventTag EVT_FAINT_ZoomActualSizeAll;

extern const wxEventType FAINT_ZoomFit;
extern CommandEventTag EVT_FAINT_ZoomFit;

extern const wxEventType FAINT_ZoomFitAll;
extern CommandEventTag EVT_FAINT_ZoomFitAll;

extern const wxEventType FAINT_ZoomActualSizeToggle;
extern CommandEventTag EVT_FAINT_ZoomActualSizeToggle;

// Event for when a control changes size, so that parent-panels can
// re-Layout
extern CommandEventTag EVT_FAINT_ControlResized;

void send_control_resized_event(wxEvtHandler*);

} // namespace

namespace faint{

enum class CopyColorMode { HEX, RGB };

} // namespace

namespace faint::events{

// Event for notifying that a text entry control has received or lost
// focus (used to disable some Python binds)
void set_focus_entry(window_t);
void on_set_focus_entry(window_t, const void_func&);
void kill_focus_entry(window_t);
void on_kill_focus_entry(window_t, const void_func&);

void queue_open_files(window_t, const FileList&);
void on_open_files(window_t, const std::function<void(const FileList&)>&);

void layer_change(window_t, Layer);
void on_layer_change(window_t, const std::function<void(Layer)>&);

void tool_change(window_t, ToolId);
void on_tool_change(window_t, const std::function<void(ToolId)>&);

void add_to_palette(window_t, const Paint&);
void on_add_to_palette(window_t, const std::function<void(const Paint&)>&);

void copy_color_string(window_t, const Color&, CopyColorMode);
void on_copy_color_string(window_t,
  const std::function<void(const Color&, CopyColorMode)>&);

void queue_request_close_faint(window_t, bool force);
void on_request_close_faint(window_t, const std::function<void(bool force)>&);

} // namespace

#endif
