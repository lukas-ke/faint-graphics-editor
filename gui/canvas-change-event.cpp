// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
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

#include "gui/canvas-change-event.hh"

namespace faint{

CanvasChangeEvent::CanvasChangeEvent(wxEventType type, const CanvasId& canvasId)
  : wxCommandEvent(type, -1),
    m_canvasId(canvasId)
{}

wxEvent* CanvasChangeEvent::Clone() const{
  return new CanvasChangeEvent(*this);
}

CanvasId CanvasChangeEvent::GetCanvasId() const{
  return m_canvasId;
}

} // namespace
