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

#ifndef FAINT_LINEAR_GRADIENT_DISPLAY_HH
#define FAINT_LINEAR_GRADIENT_DISPLAY_HH
#include "bitmap/gradient.hh"

class wxWindow;

namespace faint{

class DialogContext;
class IntSize;

class LinearGradientDisplay{
public:
  LinearGradientDisplay(wxWindow* parent, const IntSize&, DialogContext&);
  ~LinearGradientDisplay();
  wxWindow* AsWindow();
  const LinearGradient& GetGradient() const;
  void Hide();
  void SetBackgroundColor(const Color&);
  void SetStops(const color_stops_t&);
  void SetGradient(const LinearGradient&);
  void Show();

private:
  class LinearGradientDisplayImpl;
  LinearGradientDisplayImpl* m_impl;
};

} // namespace

namespace faint{ namespace events{

void on_angle_picked(LinearGradientDisplay&,
  const std::function<void()>&);

}} // namespace

#endif
