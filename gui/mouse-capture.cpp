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

#include "wx/window.h"
#include "gui/mouse-capture.hh"
#include "util-wx/bind-event.hh"

namespace faint{

MouseCapture::MouseCapture(wxWindow* window,
  const std::function<void()>& onLoss,
  const std::function<void()>& onRelease)
  : m_onCaptureLost(onLoss),
    m_onRelease(onRelease),
    m_window(window)
{
  bind(window, wxEVT_MOUSE_CAPTURE_LOST,
    [&](){
      if (m_onCaptureLost != nullptr){
        m_onCaptureLost();
        if (m_onRelease != nullptr){
          m_onRelease();
        }
      }
    });
}

MouseCapture::MouseCapture(wxWindow* w, const std::function<void()>& onLoss)
  : MouseCapture(w, onLoss, nullptr)
{}

MouseCapture::MouseCapture(wxWindow* w)
  : MouseCapture(w, nullptr)
{}

MouseCapture::~MouseCapture(){
  Release();
}

void MouseCapture::Capture(){
  if (!m_window->HasCapture()){
    m_window->CaptureMouse();
  }
}

bool MouseCapture::HasCapture() const{
  return m_window->HasCapture();
}

void MouseCapture::Release(){
  if (m_window->HasCapture()){
    m_window->ReleaseMouse();
    if (m_onRelease != nullptr){
      m_onRelease();
    }
  }
}

} // namespace
