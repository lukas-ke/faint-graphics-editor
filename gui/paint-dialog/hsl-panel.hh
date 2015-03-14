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

#ifndef FAINT_HSL_PANEL_HH
#define FAINT_HSL_PANEL_HH

class wxWindow;

namespace faint{

class Color;

class PaintPanel_HSL{
public:
  PaintPanel_HSL(wxWindow* parent);
  ~PaintPanel_HSL();
  wxWindow* AsWindow();
  Color GetColor() const;
  void SetColor(const Color&);

  PaintPanel_HSL(const PaintPanel_HSL&) = delete;
  PaintPanel_HSL& operator=(const PaintPanel_HSL&) = delete;
private:
  class PaintPanel_HSL_Impl;
  PaintPanel_HSL_Impl* m_impl;
};

} // namespace

#endif
