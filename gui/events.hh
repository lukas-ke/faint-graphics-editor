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


// PaintEvent
class PaintEvent : public wxCommandEvent{
public:
  PaintEvent(wxEventType, const Paint&);
  wxEvent* Clone() const override;
  Paint GetPaint() const;
private:
  Paint m_paint;
};

extern const wxEventType FAINT_AddToPalette;
extern const wxEventTypeTag<PaintEvent> EVT_FAINT_AddToPalette;


// ColorEvent
class ColorEvent : public wxCommandEvent{
public:
  ColorEvent(wxEventType, const Color&);
  wxEvent* Clone() const override;
  Color GetColor() const;
private:
  Color m_color;
};

extern const wxEventType FAINT_CopyColorHex;
extern const wxEventType FAINT_CopyColorRgb;
using ColorEventTag = const wxEventTypeTag<ColorEvent>;
extern const ColorEventTag EVT_FAINT_CopyColorHex;
extern const ColorEventTag EVT_FAINT_CopyColorRgb;


// ToolChangeEvent
class ToolChangeEvent : public wxCommandEvent{
public:
  ToolChangeEvent(ToolId);
  wxEvent* Clone() const override;
  ToolId GetTool() const;
private:
  ToolId m_toolId;
};

extern const wxEventType FAINT_ToolChange;
extern const wxEventTypeTag<ToolChangeEvent> EVT_FAINT_ToolChange;


// LayerChangeEvent
class LayerChangeEvent : public wxCommandEvent{
public:
  LayerChangeEvent(Layer);
  wxEvent* Clone() const override;
  Layer GetLayer() const;
private:
  Layer m_layer;
};

extern const wxEventType FAINT_LayerChange;
extern const wxEventTypeTag<LayerChangeEvent> EVT_FAINT_LayerChange;


// OpenFilesEvent
class OpenFilesEvent : public wxCommandEvent{
public:
  OpenFilesEvent(const FileList&);
  wxEvent* Clone() const override;
  const FileList& GetFileNames() const;
private:
  FileList m_files;
};

extern const wxEventType FAINT_OpenFiles;
extern const wxEventTypeTag<OpenFilesEvent> EVT_FAINT_OpenFiles;


// Event for notifying that a text entry control has received or lost
// focus (used to disable some Python binds)
extern const wxEventType FAINT_SetFocusEntryControl;
extern const wxEventTypeTag<wxCommandEvent> EVT_FAINT_SetFocusEntryControl;

extern const wxEventType FAINT_KillFocusEntryControl;
extern const wxEventTypeTag<wxCommandEvent> EVT_FAINT_KillFocusEntryControl;

} // namespace

#endif
