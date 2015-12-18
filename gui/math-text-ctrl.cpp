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

#include <sstream>
#include "wx/textctrl.h"
#include "geo/primitive.hh"
#include "gui/math-text-ctrl.hh"
#include "util-wx/bind-event.hh"
#include "util-wx/fwd-bind.hh"
#include "util-wx/gui-util.hh"
#include "util/parse-math-string.hh"
#include "util/dumb-ptr.hh" // make_wx

namespace faint{
// Event sent by MathTextCtrl when the value is changed by user entry
extern const wxEventType MATH_TEXT_CONTROL_UPDATE;
extern const wxEventTypeTag<wxCommandEvent> EVT_MATH_TEXT_CONTROL_UPDATE;

const wxEventType MATH_TEXT_CONTROL_UPDATE = wxNewEventType();
const wxEventTypeTag<wxCommandEvent> EVT_MATH_TEXT_CONTROL_UPDATE(
  MATH_TEXT_CONTROL_UPDATE);

}

namespace faint{ namespace events{
void on_value_change(MathTextCtrl* c, const coord_func& f){
  bind_fwd(c->AsWindow(), EVT_MATH_TEXT_CONTROL_UPDATE,
    [f, c](const wxCommandEvent&){
      f(c->GetValue());
    });
}

}}

namespace faint{

static std::string format_inactive(coord value, coord originalValue){
  std::stringstream ss;
  ss << value << " (" << rounded(value / originalValue * 100) << "%)";
  return ss.str();
}

std::string format_active(coord value){
  std::stringstream ss;
  ss << value;
  return ss.str();
}

class MathTextCtrl::MathTextCtrlImpl : public wxTextCtrl {
  // Using an impl-class avoids showing all public functions from
  // wxTextCtrl in the public interface of MathTextCtrl.
public:
  MathTextCtrlImpl(wxWindow* parent, coord value) :
    wxTextCtrl(parent, wxID_ANY, "", wxPoint(1,1))
  {
    m_originalValue = value;
    SetCoordValue(value, false);

    events::on_set_focus(this, [this](){
      // When focus is gained, switch to entry-mode formatted value
      SetValue(format_active(m_value));
      SelectAll();
    });

    events::on_kill_focus(this, [this](){
      // When focus is lost, display the value with a percentage relation
      // and update the current value

      coord lastValue = UpdateValue();
      SetValue(format_inactive(m_value, m_originalValue));

      if (m_value != lastValue){
        wxCommandEvent updateEvent(EVT_MATH_TEXT_CONTROL_UPDATE, GetId());
        // Clients don't know about this class, so use the
        // parent (MathTextCtrl) as the event object.
        updateEvent.SetEventObject(GetParent());
        GetEventHandler()->ProcessEvent(updateEvent);
      }
    });
  }

  void SetCoordValue(coord value, bool hasFocus){
    m_value = value;
    SetValue(hasFocus ? format_active(m_value) :
      format_inactive(m_value, m_originalValue));
  }
  coord GetCoordValue(){
    UpdateValue();
    return m_value;
  }

  coord GetOldCoordValue(){
    return m_originalValue;
  }

private:
  coord m_value;
  coord m_originalValue;

  coord UpdateValue(){
    coord lastValue = m_value;
    auto result = parse_math_string((std::string)GetValue(), m_originalValue);
    result.IfSet(
      [&](coord value){
        if (value > 0){
          m_value = value;
        }
      });
    return lastValue;
  }
};

MathTextCtrl::MathTextCtrl(wxWindow* parent, coord value){
  m_textCtrl = make_wx<MathTextCtrlImpl>(parent, value);
  SetValue(value);
}

wxWindow* MathTextCtrl::AsWindow(){
  return m_textCtrl;
}

MathTextCtrl::MathTextCtrl(wxWindow* parent,
  coord value,
  const std::function<void(coord)>& f)
  : MathTextCtrl(parent, value)
{
  events::on_value_change(this, f);
}

void MathTextCtrl::SetValue(coord value){
  m_textCtrl->SetCoordValue(value, m_textCtrl->HasFocus());
}

coord MathTextCtrl::GetValue() const{
  return m_textCtrl->GetCoordValue();
}

coord MathTextCtrl::GetOldValue() const{
  return m_textCtrl->GetOldCoordValue();
}

bool MathTextCtrl::HasFocus() const{
  return m_textCtrl->HasFocus();
}

void MathTextCtrl::FitSizeTo(const utf8_string& str){
  fit_size_to(m_textCtrl, str);
}

wxWindow* as_window(MathTextCtrl* c){
  return c->AsWindow();
}

} // namespace
