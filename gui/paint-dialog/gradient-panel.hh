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

#ifndef FAINT_GRADIENT_PANEL_HH
#define FAINT_GRADIENT_PANEL_HH
#include "bitmap/color.hh"
#include "bitmap/gradient.hh"

class wxWindow;

namespace faint{

class StatusInterface;

class PaintPanel_Gradient{
public:
  PaintPanel_Gradient(wxWindow* parent,
    const Color& bgColor,
    StatusInterface&,
    DialogContext&);
  ~PaintPanel_Gradient();
  wxWindow* AsWindow();
  Gradient GetGradient() const;
  void SetGradient(const Gradient&);

  PaintPanel_Gradient(const PaintPanel_Gradient&) = delete;
  PaintPanel_Gradient& operator=(const PaintPanel_Gradient&) = delete;
private:
  class PaintPanel_Gradient_Impl;
  PaintPanel_Gradient_Impl* m_impl;
};

} // namespace

#endif
