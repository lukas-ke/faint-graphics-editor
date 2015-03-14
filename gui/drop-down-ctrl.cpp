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

#include "gui/drop-down-ctrl.hh"
#include "util-wx/bind-event.hh"
#include "util-wx/fwd-wx.hh"

namespace faint{

class DropDown : public StringSettingControl {
public:
  DropDown(wxWindow* parent,
    const StringSetting& setting,
    const StringSource& source)
    : StringSettingControl(parent, setting),
      m_source(source)
  {
    m_choice = create_choice(this, m_source.Get());
    bind(this, wxEVT_CHOICE,
      [this](){
        SendChangeEvent();
      });
  }

  utf8_string GetValue() const override{
    return get_text(m_choice);
  }

  void SetValue(const utf8_string& s) override{
    select(m_choice, s);
  }

private:
  wxChoice* m_choice = nullptr;
  const StringSource& m_source;
};

StringSettingControl* create_drop_down(wxWindow* parent,
  const StringSetting& setting,
  const StringSource& source,
  const Tooltip& tooltip)
{
  auto* ctrl = new DropDown(parent, setting, source);
  set_tooltip(ctrl, tooltip);
  return ctrl;
}

} // namespace
