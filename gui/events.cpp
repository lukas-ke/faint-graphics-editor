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

#include "gui/events.hh"

namespace faint{

// Zoom command event definitions
const wxEventType FAINT_ZOOM_IN = wxNewEventType();
CommandEventTag EVT_FAINT_ZOOM_IN(FAINT_ZOOM_IN);

const wxEventType FAINT_ZOOM_IN_ALL = wxNewEventType();
CommandEventTag EVT_FAINT_ZOOM_IN_ALL(FAINT_ZOOM_IN_ALL);

const wxEventType FAINT_ZOOM_OUT = wxNewEventType();
CommandEventTag EVT_FAINT_ZOOM_OUT(FAINT_ZOOM_OUT);

const wxEventType FAINT_ZOOM_OUT_ALL = wxNewEventType();
CommandEventTag EVT_FAINT_ZOOM_OUT_ALL(FAINT_ZOOM_OUT_ALL);

const wxEventType FAINT_ZOOM_100 = wxNewEventType();
CommandEventTag EVT_FAINT_ZOOM_100(FAINT_ZOOM_100);

const wxEventType FAINT_ZOOM_100_ALL = wxNewEventType();
CommandEventTag EVT_FAINT_ZOOM_100_ALL(FAINT_ZOOM_100_ALL);

const wxEventType FAINT_ZOOM_FIT = wxNewEventType();
CommandEventTag EVT_FAINT_ZOOM_FIT(FAINT_ZOOM_FIT);

const wxEventType FAINT_ZOOM_FIT_ALL = wxNewEventType();
CommandEventTag EVT_FAINT_ZOOM_FIT_ALL(FAINT_ZOOM_FIT_ALL);

const wxEventType FAINT_ZOOM_100_TOGGLE = wxNewId();
CommandEventTag EVT_FAINT_ZOOM_100_TOGGLE(FAINT_ZOOM_100_TOGGLE);


// Control resized
const wxEventType FAINT_CONTROL_RESIZED = wxNewEventType();
CommandEventTag EVT_FAINT_CONTROL_RESIZED(FAINT_CONTROL_RESIZED);

void send_control_resized_event(wxEvtHandler* handler){
  wxCommandEvent sizeEvent(FAINT_CONTROL_RESIZED);
  sizeEvent.SetEventObject(handler);
  handler->ProcessEvent(sizeEvent);
}

// PaintEvent
const wxEventType FAINT_ADD_TO_PALETTE = wxNewEventType();
const wxEventTypeTag<PaintEvent> EVT_FAINT_ADD_TO_PALETTE(FAINT_ADD_TO_PALETTE);

PaintEvent::PaintEvent(wxEventType type, const Paint& paint)
  : wxCommandEvent(type, -1),
    m_paint(paint)
{}

wxEvent* PaintEvent::Clone() const{
  return new PaintEvent(*this);
}

Paint PaintEvent::GetPaint() const{
  return m_paint;
}

// ColorEvent
const wxEventType FAINT_COPY_COLOR_HEX = wxNewEventType();
const wxEventType FAINT_COPY_COLOR_RGB = wxNewEventType();
ColorEventTag EVT_FAINT_COPY_COLOR_HEX(FAINT_COPY_COLOR_HEX);
ColorEventTag EVT_FAINT_COPY_COLOR_RGB(FAINT_COPY_COLOR_RGB);

ColorEvent::ColorEvent(wxEventType type, const Color& color)
  : wxCommandEvent(type, -1),
    m_color(color)
{}

wxEvent* ColorEvent::Clone() const{
  return new ColorEvent(*this);
}

Color ColorEvent::GetColor() const{
  return m_color;
}

// ToolChangeEvent
ToolChangeEvent::ToolChangeEvent(ToolId toolId)
  : wxCommandEvent(FAINT_TOOL_CHANGE, -1),
    m_toolId(toolId)
{}

wxEvent* ToolChangeEvent::Clone() const{
  return new ToolChangeEvent(*this);
}

ToolId ToolChangeEvent::GetTool() const{
  return m_toolId;
}

const wxEventType FAINT_TOOL_CHANGE = wxNewEventType();
const wxEventTypeTag<ToolChangeEvent> EVT_FAINT_TOOL_CHANGE(FAINT_TOOL_CHANGE);


// LayerChangeEvent
LayerChangeEvent::LayerChangeEvent(Layer layer)
  : wxCommandEvent(FAINT_LAYER_CHANGE, -1),
    m_layer(layer)
{}

wxEvent* LayerChangeEvent::Clone() const{
  return new LayerChangeEvent(*this);
}

Layer LayerChangeEvent::GetLayer() const{
  return m_layer;
}
const wxEventType FAINT_LAYER_CHANGE = wxNewEventType();
const wxEventTypeTag<LayerChangeEvent> EVT_FAINT_LAYER_CHANGE(FAINT_LAYER_CHANGE);


// OpenFilesEvent
OpenFilesEvent::OpenFilesEvent(const FileList& files)
  : wxCommandEvent(FAINT_OPEN_FILES, -1),
    m_files(files)
{}

wxEvent* OpenFilesEvent::Clone() const{
  return new OpenFilesEvent(*this);
}

const FileList& OpenFilesEvent::GetFileNames() const{
  return m_files;
}

const wxEventType FAINT_OPEN_FILES = wxNewEventType();
const wxEventTypeTag<OpenFilesEvent> EVT_FAINT_OPEN_FILES(FAINT_OPEN_FILES);

const wxEventType SET_FOCUS_ENTRY_CONTROL = wxNewEventType();
CommandEventTag EVT_SET_FOCUS_ENTRY_CONTROL(SET_FOCUS_ENTRY_CONTROL);

const wxEventType KILL_FOCUS_ENTRY_CONTROL = wxNewEventType();
CommandEventTag EVT_KILL_FOCUS_ENTRY_CONTROL(KILL_FOCUS_ENTRY_CONTROL);

} // namespace
