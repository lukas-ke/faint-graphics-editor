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

#ifndef FAINT_SPIN_CTRL_HH
#define FAINT_SPIN_CTRL_HH
#include "gui/tool-setting-ctrl.hh"

namespace faint{

IntSettingCtrl* create_int_spinner(wxWindow* parent,
  const wxSize&,
  const IntSetting&,
  int value,
  const std::string& label);

FloatSettingControl* create_float_spinner(wxWindow* parent,
  const wxSize& size,
  const FloatSetting&,
  coord value,
  const std::string&);

// Alternative, only actually works with integers, but returns/accepts
// floating point. Used due to problems with wxSpinCtrlDouble.
FloatSettingControl* create_semi_float_spinner(wxWindow* parent,
  const wxSize& size,
  const FloatSetting&,
  coord value,
  const std::string&);

} // namespace

#endif
