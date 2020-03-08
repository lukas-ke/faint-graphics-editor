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

#ifndef FAINT_GRADIENT_SLIDER_HH
#define FAINT_GRADIENT_SLIDER_HH
#include <functional>
#include <memory>
#include "bitmap/gradient.hh"
#include "geo/int-size.hh"
#include "util/pick-paint.hh"

class wxWindow;

namespace faint{

class Color;
class DialogContext;

class ColorStopSliderImpl;

// Fixme: These classes hold a reference to the passed in gradient. Surprising!

class LinearGradientSlider{
public:
  LinearGradientSlider(wxWindow* parent,
    const IntSize&,
    LinearGradient&,
    const pick_color_f& pickStopColor,
    DialogContext&);
  ~LinearGradientSlider();

  wxWindow* AsWindow();
  void SetBackgroundColor(const Color&);
  void UpdateGradient();

  static const int HANDLE_WIDTH = 10;
  static const int HORIZONTAL_MARGIN = HANDLE_WIDTH / 2;
  static const int HEIGHT = 40;

private:
  std::unique_ptr<ColorStopSliderImpl> m_impl;
};

class RadialGradientSlider{
public:
  RadialGradientSlider(wxWindow* parent,
    const IntSize&,
    RadialGradient&,
    const pick_color_f& pickStopColor,
    DialogContext&);
  ~RadialGradientSlider();

  wxWindow* AsWindow();
  void SetBackgroundColor(const Color&);
  void UpdateGradient();

  static const int HANDLE_WIDTH = 10;
  static const int HORIZONTAL_MARGIN = HANDLE_WIDTH / 2;
  static const int HEIGHT = 40;
private:
  std::unique_ptr<ColorStopSliderImpl> m_impl;
};

} // namespace

namespace faint::events{

void on_gradient_slider_change(LinearGradientSlider&,
  const std::function<void()>&);

void on_gradient_slider_change(RadialGradientSlider&,
  const std::function<void()>&);

} // namespace

#endif
