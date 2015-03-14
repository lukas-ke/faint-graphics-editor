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

#include "wx/dcclient.h"
#include "app/get-art-container.hh" // Fixme: Only for cursors
#include "app/resource-id.hh"
#include "geo/angle.hh"
#include "geo/geo-func.hh"
#include "geo/line.hh"
#include "geo/measure.hh"
#include "gui/art-container.hh"
#include "gui/drag-value-ctrl.hh"
#include "text/formatting.hh"
#include "util/status-interface.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/fwd-bind.hh"
#include "util-wx/fwd-wx.hh"

namespace faint{

const wxEventType FAINT_DRAG_VALUE_CHANGE = wxNewEventType();

DragValueChangeEvent::DragValueChangeEvent()
  : wxCommandEvent(FAINT_DRAG_VALUE_CHANGE)
{}

DragValueChangeEvent* DragValueChangeEvent::Clone() const{
  return new DragValueChangeEvent();
}

const wxEventTypeTag<DragValueChangeEvent> EVT_FAINT_DRAG_VALUE_CHANGE(
  FAINT_DRAG_VALUE_CHANGE);

const wxColour g_highlightColor(wxColour(0,0,0));
const wxColour g_originalColor(wxColour(40,40,40));

static int drag_value(const IntPoint& p0,
  const IntPoint& p1,
  int value_p0,
  const IntRange& range,
  coord dampening)
{
  assert(dampening > 0);
  coord d = truncated(distance(p0, p1));
  Angle a = angle360({floated(p0), floated(p1)});
  if (Angle::Deg(135) < a && a < Angle::Deg(315)){
    d *= -1;
  }
  return range.Constrain(value_p0 - truncated(d / dampening));
}

DragValueCtrl::DragValueCtrl(wxWindow* parent,
  const IntRange& range,
  const Description& statusText,
  StatusInterface& statusInfo)
  : wxPanel(parent, wxID_ANY),
    m_currentValue(10),
    m_mouse(this, [&](){
        // On capture lost
        SetForegroundColour(g_originalColor);
        m_originValue = m_currentValue = GetDragValue();
        SendChangeEvent(m_originValue);
      },
      [&](){
        // On release
        SetCursor(get_art_container().Get(Cursor::MOVE_POINT));
      }),
    m_originValue(10),
    m_range(range),
    m_statusInfo(statusInfo),
    m_statusText(statusText.Get())
{
  SetForegroundColour(g_originalColor);
  SetCursor(get_art_container().Get(Cursor::MOVE_POINT));
  SetInitialSize(wxSize(50,40));

  events::on_mouse_left_down(this,
    [&](const IntPoint& pos){
      SetCursor(get_art_container().Get(Cursor::DRAG_SCALE));
      m_origin = m_current = pos;
      m_originValue = m_currentValue;
      SetForegroundColour(g_highlightColor);
      m_mouse.Capture();
      m_statusInfo.SetMainText("");
    });

  events::on_mouse_left_up(this,
    [&](const IntPoint& pos){
      SetForegroundColour(g_originalColor);
      Refresh();
      m_mouse.Release();

      m_originValue = m_currentValue = GetDragValue();
      auto size = get_size(this);
      if (pos.x < size.w && pos.y < size.h &&
        pos.x > 0 && pos.y > 0)
      {
        m_statusInfo.SetMainText(m_statusText);
      }
      Refresh();
      SendChangeEvent(m_originValue);
    });

  events::on_mouse_motion(this,
    [&](const IntPoint& pos){
      if (m_mouse.HasCapture()){
        m_current = pos;
        int newValue = GetDragValue();
        if (newValue != m_currentValue){
          m_currentValue = newValue;
          Update(); // Required to avoid long delay in refreshing the number
          SendChangeEvent(m_currentValue);
        }
    }});

  events::on_mouse_enter_window(this, [&](){
    if (!m_mouse.HasCapture()){
      m_statusInfo.SetMainText(m_statusText);
    }
  });

  events::on_mouse_leave_window(this, [&](){
    m_statusInfo.SetMainText("");
  });

  events::on_paint(this, [&](){
    wxPaintDC dc(this);
    wxString str(to_wx(str_int(m_currentValue)));
    wxCoord textWidth, textHeight;
    dc.GetTextExtent(str, &textWidth, &textHeight);
    wxSize winSize(GetSize());
    dc.DrawText(str, winSize.GetWidth() / 2 - textWidth / 2 ,
      winSize.GetHeight() / 2 - textHeight / 2);
  });
}

int DragValueCtrl::GetDragValue() const{
  coord dampening = wxGetKeyState(WXK_CONTROL) ?
    10.0 : 3.0;
  return drag_value(m_current, m_origin, m_originValue, m_range, dampening);
}

void DragValueCtrl::SendChangeEvent(int value){
  DragValueChangeEvent event;
  event.SetEventObject(this);
  event.SetInt(value);
  ProcessEvent(event);
}

void DragValueCtrl::SetValue(int value){
  m_currentValue = value;
  Refresh();
}

} // namespace
