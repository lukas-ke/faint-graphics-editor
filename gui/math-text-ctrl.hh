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

#ifndef FAINT_MATH_TEXT_CTRL_HH
#define FAINT_MATH_TEXT_CTRL_HH
#include "wx/panel.h" // Fixme: Don't inherit, hide in impl.
#include "text/utf8-string.hh"
#include "util-wx/fwd-bind.hh"

namespace faint{ class MathTextCtrl; }

namespace faint{ namespace events{

// Event handler for when the MathTextCtrl:s value is changed by user
// entry.
using coord_func = std::function<void(coord)>;
void on_value_change(MathTextCtrl*, const coord_func&);

}} // namespace

namespace faint{

class MathTextCtrl : public wxPanel{
public:
  MathTextCtrl(wxWindow* parent, coord value);
  MathTextCtrl(wxWindow* parent, coord value, const events::coord_func& onChange);
  void FitSizeTo(const utf8_string&);
  void SetValue(coord);
  coord GetValue() const;
  coord GetOldValue() const;
  bool HasFocus() const override;
private:
  class MathTextCtrlImpl;
  MathTextCtrlImpl* m_textCtrl;
};

} // namespace

#endif
