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

namespace faint{

using CommandEventTag = const wxEventTypeTag<wxCommandEvent>;

// Zoom command events declarations
extern const wxEventType FAINT_ZOOM_IN;
extern CommandEventTag EVT_FAINT_ZOOM_IN;

extern const wxEventType FAINT_ZOOM_IN_ALL;
extern CommandEventTag EVT_FAINT_ZOOM_IN_ALL;

extern const wxEventType FAINT_ZOOM_OUT;
extern CommandEventTag EVT_FAINT_ZOOM_OUT;

extern const wxEventType FAINT_ZOOM_OUT_ALL;
extern CommandEventTag EVT_FAINT_ZOOM_OUT_ALL;

extern const wxEventType FAINT_ZOOM_100;
extern CommandEventTag EVT_FAINT_ZOOM_100;

extern const wxEventType FAINT_ZOOM_100_ALL;
extern CommandEventTag EVT_FAINT_ZOOM_100_ALL;

extern const wxEventType FAINT_ZOOM_FIT;
extern CommandEventTag EVT_FAINT_ZOOM_FIT;

extern const wxEventType FAINT_ZOOM_FIT_ALL;
extern CommandEventTag EVT_FAINT_ZOOM_FIT_ALL;

extern const wxEventType FAINT_ZOOM_100_TOGGLE;
extern CommandEventTag EVT_FAINT_ZOOM_100_TOGGLE;

// Event for when a control changes size, so that parent-panels can
// re-Layout
extern CommandEventTag EVT_FAINT_CONTROL_RESIZED;

void send_control_resized_event(wxEvtHandler*);

// PaintEvent
class PaintEvent : public wxCommandEvent{
public:
  PaintEvent(wxEventType, const Paint&);
  wxEvent* Clone() const override;
  Paint GetPaint() const;
private:
  Paint m_paint;
};

extern const wxEventType FAINT_ADD_TO_PALETTE;
extern const wxEventTypeTag<PaintEvent> EVT_FAINT_ADD_TO_PALETTE;

// ColorEvent
class ColorEvent : public wxCommandEvent{
public:
  ColorEvent(wxEventType, const Color&);
  wxEvent* Clone() const override;
  Color GetColor() const;
private:
  Color m_color;
};

extern const wxEventType FAINT_COPY_COLOR_HEX;
extern const wxEventType FAINT_COPY_COLOR_RGB;
using ColorEventTag = const wxEventTypeTag<ColorEvent>;
extern const ColorEventTag EVT_FAINT_COPY_COLOR_HEX;
extern const ColorEventTag EVT_FAINT_COPY_COLOR_RGB;

// ToolChangeEvent
class ToolChangeEvent : public wxCommandEvent{
public:
  ToolChangeEvent(ToolId);
  wxEvent* Clone() const override;
  ToolId GetTool() const;
private:
  ToolId m_toolId;
};
extern const wxEventType FAINT_TOOL_CHANGE;
extern const wxEventTypeTag<ToolChangeEvent> EVT_FAINT_TOOL_CHANGE;


// LayerChangeEvent
class LayerChangeEvent : public wxCommandEvent{
public:
  LayerChangeEvent(Layer);
  wxEvent* Clone() const override;
  Layer GetLayer() const;
private:
  Layer m_layer;
};

extern const wxEventType FAINT_LAYER_CHANGE;
extern const wxEventTypeTag<LayerChangeEvent> EVT_FAINT_LAYER_CHANGE;

// OpenFilesEvent
class OpenFilesEvent : public wxCommandEvent{
public:
  OpenFilesEvent(const FileList&);
  wxEvent* Clone() const override;
  const FileList& GetFileNames() const;
private:
  FileList m_files;
};

extern const wxEventType FAINT_OPEN_FILES;
extern const wxEventTypeTag<OpenFilesEvent> EVT_FAINT_OPEN_FILES;

// Event for notifying that a text entry control has received or lost
// focus (used to disable some Python binds)
extern const wxEventType SET_FOCUS_ENTRY_CONTROL;
extern const wxEventTypeTag<wxCommandEvent> EVT_SET_FOCUS_ENTRY_CONTROL;

extern const wxEventType KILL_FOCUS_ENTRY_CONTROL;
extern const wxEventTypeTag<wxCommandEvent> EVT_KILL_FOCUS_ENTRY_CONTROL;

// Event for notifying that a text entry control has received or lost
// focus (used to disable some Python binds)
extern const wxEventType SET_FOCUS_ENTRY_CONTROL;
extern const wxEventTypeTag<wxCommandEvent> EVT_SET_FOCUS_ENTRY_CONTROL;

extern const wxEventType KILL_FOCUS_ENTRY_CONTROL;
extern const wxEventTypeTag<wxCommandEvent> EVT_KILL_FOCUS_ENTRY_CONTROL;

} // namespace

#endif
