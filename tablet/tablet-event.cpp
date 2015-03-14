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

#include "tablet/tablet-event.hh"

namespace faint{

TabletEvent::TabletEvent(int x, int y, int pressure, int cursor)
  : wxCommandEvent(TABLET_EVENT),
    m_x(x),
    m_y(y),
    m_pressure(pressure),
    m_cursor(cursor)
{}

wxEvent* TabletEvent::Clone() const{
  return new TabletEvent(m_x, m_y, m_pressure, m_cursor);
}

int TabletEvent::GetX() const{
  return m_x;
}

int TabletEvent::GetY() const{
  return m_x;
}

int TabletEvent::GetPressure() const{
  return m_pressure;
}

int TabletEvent::GetCursor() const{
  return m_cursor;
}

const wxEventType TABLET_EVENT = wxNewEventType();
const wxEventTypeTag<TabletEvent> EVT_TABLET(TABLET_EVENT);

} // namespace
