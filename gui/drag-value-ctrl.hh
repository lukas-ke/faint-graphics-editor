// -*- coding: us-ascii-unix -*-
// Copyright 2013 Lukas Kemmer
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

#ifndef FAINT_DRAG_VALUE_CTRL_HH
#define FAINT_DRAG_VALUE_CTRL_HH
#include "wx/event.h"
#include "wx/panel.h"
#include "geo/int-point.hh"
#include "geo/int-range.hh"
#include "gui/gui-string-types.hh"
#include "gui/mouse-capture.hh"

namespace faint{

class StatusInterface;

class DragValueChangeEvent : public wxCommandEvent{
public:
  DragValueChangeEvent();
  DragValueChangeEvent* Clone() const override;
};

extern const wxEventTypeTag<DragValueChangeEvent> EVT_FAINT_DRAG_VALUE_CHANGE;

class DragValueCtrl : public wxPanel {
public:
  // Note: Does not use tooltips as they tended to obscure the value
  DragValueCtrl(wxWindow* parent,
    const IntRange&,
    const Description&,
    StatusInterface&);

  void SetValue(int);

private:
  int GetDragValue() const;
  void SendChangeEvent(int);
  IntPoint m_current;
  int m_currentValue;
  MouseCapture m_mouse;
  IntPoint m_origin;
  int m_originValue;
  IntRange m_range;
  StatusInterface& m_statusInfo;
  utf8_string m_statusText;
};

} // namespace

#endif
