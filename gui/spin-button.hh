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

#ifndef FAINT_SPIN_BUTTON_HH
#define FAINT_SPIN_BUTTON_HH
#include <string>

class wxWindow;

namespace faint{
class SpinButtonImpl;
class IntSize;

class SpinButton{
  // A wrapper around a wxWidgets Spinbutton
public:
  SpinButton(wxWindow* parent, const IntSize&, const std::string& toolTip);
  wxWindow* GetRaw();
private:
  SpinButton(const SpinButton&);
  SpinButtonImpl* m_impl;
};

} // namespace

#endif
