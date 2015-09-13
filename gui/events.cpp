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


} // namespace

namespace faint{ namespace events{

// Entry-control focus events
// --------------------------
const wxEventType FAINT_SetFocusEntryControl = wxNewEventType();
CommandEventTag EVT_FAINT_SetFocusEntryControl(FAINT_SetFocusEntryControl);

const wxEventType FAINT_KillFocusEntryControl = wxNewEventType();
CommandEventTag EVT_FAINT_KillFocusEntryControl(FAINT_KillFocusEntryControl);

void set_focus_entry(window_t w){
  wxCommandEvent newEvent(EVT_FAINT_SetFocusEntryControl, wxID_ANY);
  newEvent.SetEventObject(w.w);
  w.w->GetEventHandler()->ProcessEvent(newEvent);
}

void on_set_focus_entry(window_t w, const void_func& f){
  bind(w.w, EVT_FAINT_SetFocusEntryControl, f);
}

void kill_focus_entry(window_t w){
  wxCommandEvent newEvent(EVT_FAINT_KillFocusEntryControl, wxID_ANY);
  newEvent.SetEventObject(w.w);
  w.w->GetEventHandler()->ProcessEvent(newEvent);
}

void on_kill_focus_entry(window_t w, const void_func& f){
  bind(w.w, EVT_FAINT_KillFocusEntryControl, f);
}

// OpenFilesEvent
// --------------
const wxEventType FAINT_OpenFiles = wxNewEventType();

class OpenFilesEvent : public wxCommandEvent{
public:
  OpenFilesEvent(const FileList& files)
    : wxCommandEvent(FAINT_OpenFiles, -1),
      m_files(files)
  {}

  wxEvent* Clone() const override{
    return new OpenFilesEvent(*this);
  }

  const FileList& GetFileNames() const{
    return m_files;
  }
private:
  FileList m_files;
};

const wxEventTypeTag<OpenFilesEvent> EVT_FAINT_OpenFiles(FAINT_OpenFiles);

void queue_open_files(window_t w, const FileList& files){
  w.w->GetEventHandler()->QueueEvent(new OpenFilesEvent(files));
}

void on_open_files(window_t w, const std::function<void(const FileList&)>& f){
  bind_fwd(w.w, EVT_FAINT_OpenFiles,
    [f](const OpenFilesEvent& e){
      f(e.GetFileNames());
  });
}

// LayerChangeEvent
// ----------------
const wxEventType FAINT_LayerChange = wxNewEventType();

class LayerChangeEvent : public wxCommandEvent{
public:
  LayerChangeEvent(Layer layer)
  : wxCommandEvent(FAINT_LayerChange, -1),
    m_layer(layer)
  {}
  wxEvent* Clone() const override{
    return new LayerChangeEvent(*this);
  }
  Layer GetLayer() const{
    return m_layer;
  }
private:
  Layer m_layer;
};

const wxEventTypeTag<LayerChangeEvent> EVT_FAINT_LayerChange(FAINT_LayerChange);

void layer_change(window_t w, Layer layer){
  LayerChangeEvent newEvent(layer);
  newEvent.SetEventObject(w.w);
  w.w->GetEventHandler()->ProcessEvent(newEvent);
}

void on_layer_change(window_t w, const std::function<void(Layer)>& f){
  bind_fwd(w.w, EVT_FAINT_LayerChange,
    [f](const LayerChangeEvent& e){
      f(e.GetLayer());
    });
}

// ToolChangeEvent
// ----------------
const wxEventType FAINT_ToolChange = wxNewEventType();

class ToolChangeEvent : public wxCommandEvent{
public:
  ToolChangeEvent(ToolId toolId)
    : wxCommandEvent(FAINT_ToolChange, -1),
      m_toolId(toolId)
  {}

  wxEvent* Clone() const override{
    return new ToolChangeEvent(*this);
  }

  ToolId GetTool() const{
    return m_toolId;
  }
private:
  ToolId m_toolId;
};

const wxEventTypeTag<ToolChangeEvent> EVT_FAINT_ToolChange(FAINT_ToolChange);

void tool_change(window_t w, ToolId toolId){
  ToolChangeEvent newEvent(toolId);
  newEvent.SetEventObject(w.w);
  w.w->GetEventHandler()->ProcessEvent(newEvent);
}

void on_tool_change(window_t w, const std::function<void(ToolId)>& f){
  bind_fwd(w.w, EVT_FAINT_ToolChange,
    [f](const ToolChangeEvent& e){
      f(e.GetTool());
    });
}

// PaintEvent
// ----------
const wxEventType FAINT_AddToPalette = wxNewEventType();

class PaintEvent : public wxCommandEvent{
public:
  PaintEvent(wxEventType type, const Paint& paint)
    : wxCommandEvent(type, -1),
      m_paint(paint)
  {}

  wxEvent* Clone() const override{
    return new PaintEvent(*this);
  }

  Paint GetPaint() const{
    return m_paint;
  }
private:
  Paint m_paint;
};

const wxEventTypeTag<PaintEvent> EVT_FAINT_AddToPalette(FAINT_AddToPalette);

void add_to_palette(window_t w, const Paint& paint){
  PaintEvent newEvent(FAINT_AddToPalette, paint);
  newEvent.SetEventObject(w.w);
  w.w->GetEventHandler()->ProcessEvent(newEvent);
}

void on_add_to_palette(window_t w, const std::function<void(const Paint&)>& f){
  bind_fwd(w.w, EVT_FAINT_AddToPalette,
    [f](const PaintEvent& e){
      f(e.GetPaint());
    });
}

}} // namespace
