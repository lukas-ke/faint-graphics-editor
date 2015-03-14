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

#ifndef FAINT_BIND_EVENT_HH
#define FAINT_BIND_EVENT_HH
#include "wx/event.h"
#include "geo/range.hh"

namespace faint{

template<typename FUNC>
void bind_menu(wxEvtHandler& handler, int menuItemId, FUNC f){
  handler.Bind(wxEVT_COMMAND_MENU_SELECTED,
    [f](const wxCommandEvent&){
      f();
    },
    menuItemId, wxID_ANY, nullptr);
}

template<typename WindowType, typename FUNC>
void bind_menu(WindowType* w, int menuItemId, FUNC f){
  w->Bind(wxEVT_COMMAND_MENU_SELECTED,
    [f](const wxCommandEvent&){
      f();
    },
    menuItemId, wxID_ANY, nullptr);
}

template<typename WindowType, typename FUNC>
void bind_menu_all(WindowType* w, FUNC f){
  w->Bind(wxEVT_COMMAND_MENU_SELECTED,
    [f](const wxCommandEvent& e){
      f(e.GetId());
    },
    wxID_ANY, wxID_ANY, nullptr);
}

// Binds an event to a zero-argument function.
template<typename FUNC>
void bind_menu(wxEvtHandler& handler, const ClosedIntRange& menuIdRange, FUNC f){
  handler.Bind(wxEVT_COMMAND_MENU_SELECTED,
    [f](const wxCommandEvent& e){
      f(e.GetId());
    },
    menuIdRange.Lower(), menuIdRange.Upper(),
    nullptr);
}

template<typename FUNC>
void bind_menu_check(wxEvtHandler& handler, int menuItemId, FUNC f){
  handler.Bind(wxEVT_COMMAND_MENU_SELECTED,
    [f](const wxCommandEvent& e){
      f(e.IsChecked());
    },
    menuItemId, wxID_ANY, nullptr);
}

// Allows binding two events to the same functor
template<typename EventTag, typename FUNC>
void bind(const EventTag& e1,
  const EventTag& e2,
  wxEvtHandler* eventHandler,
  const FUNC& onEvent)
{
  eventHandler->Bind(e1, onEvent, wxID_ANY, wxID_ANY, nullptr);
  eventHandler->Bind(e2, onEvent, wxID_ANY, wxID_ANY, nullptr);
}

// Allows binding an event to the same functor for multiple controls
template<typename EventTag, typename FUNC>
void bind(const EventTag& e,
  wxEvtHandler* handler1,
  wxEvtHandler* handler2,
  const FUNC& onEvent)
{
  handler1->Bind(e, onEvent, wxID_ANY, wxID_ANY, nullptr);
  handler2->Bind(e, onEvent, wxID_ANY, wxID_ANY, nullptr);
}

// Binds an event to a zero-argument function.
template<typename WindowType, typename T, typename FUNC>
WindowType* bind(WindowType* w, const wxEventTypeTag<T>& tag, FUNC f){
  w->Bind(tag,
    [f](const T&){
      f();
    },
    wxID_ANY, wxID_ANY, nullptr);
  return w;
}

// Binds an event to a zero-argument function.
template<typename WindowType, typename T, typename FUNC>
WindowType* bind_skip(WindowType* w, const wxEventTypeTag<T>& tag, FUNC f){
  w->Bind(tag,
    [f](T& e){
      f();
      e.Skip();
    },
    wxID_ANY, wxID_ANY, nullptr);
  return w;
}

// Binds an event to a handler (just a forward to a normal Bind)
template<typename WindowType, typename T, typename FUNC>
WindowType* bind_fwd(WindowType* w, const wxEventTypeTag<T>& tag, FUNC f){
  w->Bind(tag, f, wxID_ANY, wxID_ANY, nullptr);
  return w;
}

template<typename WindowType, typename T, typename FUNC>
WindowType* bind_fwd(WindowType* w,
  const wxEventTypeTag<T>& tag1,
  const wxEventTypeTag<T>& tag2,
  FUNC f)
{
  w->Bind(tag1, f, wxID_ANY, wxID_ANY, nullptr);
  w->Bind(tag2, f, wxID_ANY, wxID_ANY, nullptr);
  return w;
}

} // namespace

#endif
