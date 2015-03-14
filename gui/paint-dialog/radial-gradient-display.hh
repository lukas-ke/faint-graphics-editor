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

#ifndef FAINT_RADIAL_GRADIENT_DISPLAY_HH
#define FAINT_RADIAL_GRADIENT_DISPLAY_HH
#include "bitmap/gradient.hh"

class wxWindow;

namespace faint{

class DialogContext;
class IntSize;

class RadialGradientDisplay{
public:
  RadialGradientDisplay(wxWindow* parent, const IntSize&, DialogContext&);
  ~RadialGradientDisplay();
  wxWindow* AsWindow();
  void Hide();
  const RadialGradient& GetGradient() const;
  void SetBackgroundColor(const Color&);
  void SetGradient(const RadialGradient&);
  void SetStops(const color_stops_t&);
  void Show();

private:
  class RadialGradientDisplayImpl;
  RadialGradientDisplayImpl* m_impl;
};

} // namespace

#endif
