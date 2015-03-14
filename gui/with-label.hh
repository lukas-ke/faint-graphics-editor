// -*- coding: us-ascii-unix -*-
// Copyright 2015 Lukas Kemmer
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

#ifndef FAINT_WITH_LABEL_HH
#define FAINT_WITH_LABEL_HH
#include <functional>
#include "wx/window.h" // fixme
#include "wx/panel.h" // fixme
#include "wx/sizer.h" // fixme
#include "wx/stattext.h" // fixme
#include "gui/layout.hh"

namespace faint{

enum class LabelPos{
  LEFT,
  BELOW
};

inline int layout_orientation(LabelPos pos){
  switch (pos){
  case LabelPos::LEFT:
    return wxHORIZONTAL;
  case LabelPos::BELOW:
    return wxVERTICAL;
  }
  assert(false);
  return wxHORIZONTAL;
}

template<typename T>
class WithLabel : public wxPanel {
public:
  template<typename ...Args>
  WithLabel(wxWindow* parent, LabelPos pos, const wxString& label, Args&& ... args)
    : wxPanel(parent)
  {
    wxSizer* sz = new wxBoxSizer(layout_orientation(pos));
    m_label = new wxStaticText(this, wxID_ANY, label);
    m_window = new T(this, std::forward<Args>(args)...);

    if (pos == LabelPos::LEFT){
      sz->Add(m_label, 0, wxRIGHT|wxALIGN_CENTER, labelSpacing);
      sz->Add(m_window);
    }
    else if (pos == LabelPos::BELOW){
      sz->Add(m_window);
      sz->Add(m_label, 0, wxALIGN_CENTER);
    }
    SetSizerAndFit(sz);
  }

  WithLabel(wxWindow* parent, LabelPos pos, const wxString& label,
    std::function<T*(wxWindow*)> f)
    : wxPanel(parent)
  {
    wxSizer* sz = new wxBoxSizer(layout_orientation(pos));
    m_label = new wxStaticText(this, wxID_ANY, label);
    m_window = f(this);

    if (pos == LabelPos::LEFT){
      sz->Add(m_label, 0, wxRIGHT|wxALIGN_CENTER, labelSpacing);
      sz->Add(m_window, 1, wxEXPAND);
    }
    else if (pos == LabelPos::BELOW){
      sz->Add(m_window, 1, wxEXPAND);
      sz->Add(m_label, 0, wxALIGN_CENTER);
    }
    SetSizerAndFit(sz);
  }

  T& GetWindow(){
    return *m_window;
  }

  void HideLabel(){
    m_label->Hide();
  }

  void ShowLabel(){
    m_label->Show();
  }

  operator T&(){
    return *m_window;
  }

private:
  wxStaticText* m_label;
  T* m_window;
};

template<typename T, typename ...Args>
WithLabel<T>* with_label(LabelPos pos,
  wxWindow* parent,
  const wxString& label,
  Args&& ...args)
{
  return new WithLabel<T>(parent, pos, label, std::forward<Args>(args)...);
}

template<typename T>
auto with_label_f(LabelPos pos,
  wxWindow* parent,
  const wxString& label,
  std::function<T*(wxWindow*)> f)
{
  return new WithLabel<T>(parent, pos, label, f);
}

template<typename T, typename ...Args>
WithLabel<T>* label_left(Args&& ...args){
  return with_label<T>(LabelPos::LEFT, std::forward<Args>(args)...);
}

template<typename T, typename ...Args>
WithLabel<T>* label_below(Args&& ...args){
  return with_label<T>(LabelPos::BELOW, std::forward<Args>(args)...);
}

} // namespace

#endif
