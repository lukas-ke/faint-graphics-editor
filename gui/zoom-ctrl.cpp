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

#include <sstream>
#include "wx/panel.h"
#include "wx/sizer.h"
#include "wx/stattext.h"
#include "gui/events.hh"
#include "gui/status-button.hh"
#include "gui/zoom-ctrl.hh"
#include "util/zoom-level.hh"
#include "util-wx/fwd-bind.hh"

namespace faint{

#ifdef __WXMSW__
#define ZOOMCONTROL_BORDER_STYLE wxBORDER_THEME
#else
#define ZOOMCONTROL_BORDER_STYLE wxBORDER_NONE
#endif

void send_zoom_event(ZoomCtrlImpl* zoomControl, int eventId);

class ZoomCtrlImpl : public wxPanel {
public:
  ZoomCtrlImpl(wxWindow* parent, StatusInterface& status)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxTAB_TRAVERSAL|ZOOMCONTROL_BORDER_STYLE),
      m_btnZoomFitOr100(nullptr),
      m_btnZoomIn(nullptr),
      m_btnZoomOut(nullptr),
      m_currentZoomText(nullptr),
      m_fit(false)
  {
    wxSizer* hSizer = new wxBoxSizer(wxHORIZONTAL);
    m_currentZoomText = new wxStaticText(this, wxID_ANY, "####",
      wxDefaultPosition, wxSize(40,-1), wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
    hSizer->Add(m_currentZoomText, 0, wxALIGN_CENTER_VERTICAL);

    wxSizer* vSizer = new wxBoxSizer(wxVERTICAL);

    m_btnZoomIn = new StatusButton(this, wxSize(40,25), status,
      "+",
      Tooltip("Zoom In"),
      Description("Click to Zoom In, Ctrl=All Images"));
    vSizer->Add(m_btnZoomIn);

    m_btnZoomOut = new StatusButton(this, m_btnZoomIn->GetSize(), status,
      "-", Tooltip("Zoom Out"),
      Description("Click to Zoom Out, Ctrl=All Images"));
    vSizer->Add(m_btnZoomOut);

    hSizer->Add(vSizer, 0, wxEXPAND);

    m_btnZoomFitOr100 = new StatusButton(this, wxSize(60,50), status,
      "1:1", Tooltip(""), Description(""));
    hSizer->Add(m_btnZoomFitOr100,
      1,
      wxEXPAND);
    SetSizerAndFit(hSizer);
    UpdateZoom(ZoomLevel());

    events::on_button(m_btnZoomIn, [&](){
      bool ctrl = wxGetKeyState(WXK_CONTROL);
      send_zoom_event(this, ctrl ? FAINT_ZOOM_IN_ALL : FAINT_ZOOM_IN);
    });

    events::on_button(m_btnZoomOut, [&](){
      bool ctrl = wxGetKeyState(WXK_CONTROL);
      send_zoom_event(this, ctrl ? FAINT_ZOOM_OUT_ALL : FAINT_ZOOM_OUT);
    });

    events::on_button(m_btnZoomFitOr100, [&](){
      bool ctrl = wxGetKeyState(WXK_CONTROL);
      if (m_fit){
        send_zoom_event(this, ctrl ? FAINT_ZOOM_FIT_ALL : FAINT_ZOOM_FIT);
      }
      else{
        send_zoom_event(this, ctrl ? FAINT_ZOOM_100_ALL : FAINT_ZOOM_100);
      }
    });
  }

  void UpdateZoom(const ZoomLevel& zoom){
    std::stringstream ss;
    if (zoom.GetScaleFactor() >= 1.0f){
      ss << zoom.GetScaleFactor() << ":1";
    }
    else {
      ss << zoom.GetPercentage() << " %";
    }
    if (m_currentZoomText->GetLabel() == ss.str()){
      // Reduces flicker in MSW
      return;
    }
    m_currentZoomText->SetLabel(ss.str());
    if (zoom.GetPercentage() == 100){
      m_btnZoomFitOr100->UpdateText("Fit",
        Tooltip("Zoom to Fit"),
        Description("Click to fit image in view, Ctrl=All images"));
      m_fit = true;
    }
    else {
      m_btnZoomFitOr100->UpdateText("1:1",
        Tooltip("Use Actual Size"),
        Description("Click to show image at 1:1, Ctrl=All images"));
      m_fit = false;
    }
    m_btnZoomIn->Enable(!zoom.AtMax());
    m_btnZoomOut->Enable(!zoom.AtMin());
  }
private:
  StatusButton* m_btnZoomFitOr100;
  StatusButton* m_btnZoomIn;
  StatusButton* m_btnZoomOut;
  wxStaticText* m_currentZoomText;
  bool m_fit;
};

void send_zoom_event(ZoomCtrlImpl* zoomControl, int eventId){
  wxCommandEvent zoomEvent(wxEVT_COMMAND_MENU_SELECTED, eventId);
  zoomEvent.SetEventObject(zoomControl);
  zoomControl->GetEventHandler()->ProcessEvent(zoomEvent);
}

ZoomCtrl::ZoomCtrl(wxWindow* parent, StatusInterface& status)
  : m_impl(nullptr)
{
  m_impl = new ZoomCtrlImpl(parent, status);
}

ZoomCtrl::~ZoomCtrl(){
  m_impl = nullptr; // Deletion is handled by wxWidgets.
}

wxWindow* ZoomCtrl::AsWindow(){
  return m_impl;
}

void ZoomCtrl::UpdateZoom(const ZoomLevel& zoom){
  m_impl->UpdateZoom(zoom);
}

} // namespace
