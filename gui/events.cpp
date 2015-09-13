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

#include "wx/window.h"
#include "gui/events.hh"
#include "util-wx/bind-event.hh"

namespace faint{

// Zoom command event definitions
const wxEventType FAINT_ZoomIn = wxNewEventType();
CommandEventTag EVT_FAINT_ZoomIn(FAINT_ZoomIn);

const wxEventType FAINT_ZoomInAll = wxNewEventType();
CommandEventTag EVT_FAINT_ZoomInAll(FAINT_ZoomInAll);

const wxEventType FAINT_ZoomOut = wxNewEventType();
CommandEventTag EVT_FAINT_ZoomOut(FAINT_ZoomOut);

const wxEventType FAINT_ZoomOutAll = wxNewEventType();
CommandEventTag EVT_FAINT_ZoomOutAll(FAINT_ZoomOutAll);

const wxEventType FAINT_ZoomActualSize = wxNewEventType();
CommandEventTag EVT_FAINT_ZoomActualSize(FAINT_ZoomActualSize);

const wxEventType FAINT_ZoomActualSizeToggle = wxNewId();
CommandEventTag EVT_FAINT_ZoomActualSizeToggle(FAINT_ZoomActualSizeToggle);

const wxEventType FAINT_ZoomActualSizeAll = wxNewEventType();
CommandEventTag EVT_FAINT_ZoomActualSizeAll(FAINT_ZoomActualSizeAll);

const wxEventType FAINT_ZoomFit = wxNewEventType();
CommandEventTag EVT_FAINT_ZoomFit(FAINT_ZoomFit);

const wxEventType FAINT_ZoomFitAll = wxNewEventType();
CommandEventTag EVT_FAINT_ZoomFitAll(FAINT_ZoomFitAll);


// Control resized
const wxEventType FAINT_ControlResized = wxNewEventType();
CommandEventTag EVT_FAINT_ControlResized(FAINT_ControlResized);

void send_control_resized_event(wxEvtHandler* handler){
  wxCommandEvent sizeEvent(FAINT_ControlResized);
  sizeEvent.SetEventObject(handler);
  handler->ProcessEvent(sizeEvent);
}

// PaintEvent
const wxEventType FAINT_AddToPalette = wxNewEventType();
const wxEventTypeTag<PaintEvent> EVT_FAINT_AddToPalette(FAINT_AddToPalette);

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
const wxEventType FAINT_CopyColorHex = wxNewEventType();
const wxEventType FAINT_CopyColorRgb = wxNewEventType();
ColorEventTag EVT_FAINT_CopyColorHex(FAINT_CopyColorHex);
ColorEventTag EVT_FAINT_CopyColorRgb(FAINT_CopyColorRgb);

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
  : wxCommandEvent(FAINT_ToolChange, -1),
    m_toolId(toolId)
{}

wxEvent* ToolChangeEvent::Clone() const{
  return new ToolChangeEvent(*this);
}

ToolId ToolChangeEvent::GetTool() const{
  return m_toolId;
}

const wxEventType FAINT_ToolChange = wxNewEventType();
const wxEventTypeTag<ToolChangeEvent> EVT_FAINT_ToolChange(FAINT_ToolChange);


// LayerChangeEvent
LayerChangeEvent::LayerChangeEvent(Layer layer)
  : wxCommandEvent(FAINT_LayerChange, -1),
    m_layer(layer)
{}

wxEvent* LayerChangeEvent::Clone() const{
  return new LayerChangeEvent(*this);
}

Layer LayerChangeEvent::GetLayer() const{
  return m_layer;
}

const wxEventType FAINT_LayerChange = wxNewEventType();
const wxEventTypeTag<LayerChangeEvent> EVT_FAINT_LayerChange(FAINT_LayerChange);


// OpenFilesEvent
OpenFilesEvent::OpenFilesEvent(const FileList& files)
  : wxCommandEvent(FAINT_OpenFiles, -1),
    m_files(files)
{}

wxEvent* OpenFilesEvent::Clone() const{
  return new OpenFilesEvent(*this);
}

const FileList& OpenFilesEvent::GetFileNames() const{
  return m_files;
}

const wxEventType FAINT_OpenFiles = wxNewEventType();
const wxEventTypeTag<OpenFilesEvent> EVT_FAINT_OpenFiles(FAINT_OpenFiles);

} // namespace

namespace faint{ namespace events{

const wxEventType FAINT_SetFocusEntryControl = wxNewEventType();
CommandEventTag EVT_FAINT_SetFocusEntryControl(FAINT_SetFocusEntryControl);

const wxEventType FAINT_KillFocusEntryControl = wxNewEventType();
CommandEventTag EVT_FAINT_KillFocusEntryControl(FAINT_KillFocusEntryControl);

void on_kill_focus_entry(window_t w, const void_func& f){
  bind(w.w, EVT_FAINT_KillFocusEntryControl, f);
}

void on_set_focus_entry(window_t w, const void_func& f){
  bind(w.w, EVT_FAINT_SetFocusEntryControl, f);
}

void on_set_focus_entry_skip(window_t w, const void_func& f){
  bind_fwd(w.w, EVT_FAINT_SetFocusEntryControl,
    [f](wxCommandEvent& e){
      f();
      e.Skip();
    });
}

void set_focus_entry(window_t w){
  wxCommandEvent newEvent(EVT_FAINT_SetFocusEntryControl, wxID_ANY);
  newEvent.SetEventObject(w.w);
  w.w->GetEventHandler()->ProcessEvent(newEvent);
}

void kill_focus_entry(window_t w){
  wxCommandEvent newEvent(EVT_FAINT_KillFocusEntryControl, wxID_ANY);
  newEvent.SetEventObject(w.w);
  w.w->GetEventHandler()->ProcessEvent(newEvent);
}

}} // namespace
