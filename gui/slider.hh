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

#ifndef FAINT_SLIDER_HH
#define FAINT_SLIDER_HH
#include "wx/panel.h"  // Fixme
#include <functional>
#include "gui/slider-common.hh"
#include "geo/range.hh"
#include "util-wx/fwd-bind.hh"

namespace faint{

class Slider : public wxPanel{
  // Fixme: Hide this class in impl
public:
  Slider(wxWindow* parent);
  virtual int GetValue() const = 0;
  virtual void SetBackground(const SliderBackground&) = 0;
  virtual void SetValue(int) = 0;
};

class SliderMarker{
  // Interface for slider position indicators.
public:
  virtual ~SliderMarker() = default;
  virtual SliderMarker* Clone() const = 0;
  virtual void Draw(Bitmap&, SliderDir, const IntSize&, int pos) const = 0;
};

class LineSliderMarker : public SliderMarker{
  // A line indicating a slider position.
public:
  LineSliderMarker* Clone() const override;
  void Draw(Bitmap&, SliderDir, const IntSize&, int) const override;
};

class BorderedSliderMarker : public SliderMarker{
  // A bordered line indicating a slider position.
public:
  BorderedSliderMarker* Clone() const override;
  void Draw(Bitmap&, SliderDir, const IntSize&, int) const override;
};

// Creates a horizontal or vertical slider control
Slider* create_slider(wxWindow* parent,
  const BoundedInt&,
  SliderDir,
  const SliderMarker&,
  const SliderBackground&,
  const IntSize& initialSize);

Slider* create_slider(wxWindow* parent,
  const BoundedInt&,
  SliderDir,
  const SliderBackground&,
  const IntSize& initialSize);

Slider* create_slider(wxWindow* parent,
  const BoundedInt& values,
  SliderDir dir,
  const SliderMarker& marker,
  const SliderBackground& background,
  const IntSize& initialSize,
  const std::function<void()>& onChange);

Slider* create_slider(wxWindow* parent,
  const BoundedInt& values,
  SliderDir dir,
  const SliderBackground& background,
  const IntSize& initialSize,
  const std::function<void()>& onChange);

Slider* create_slider(wxWindow* parent,
  const BoundedInt& values,
  SliderDir dir,
  const SliderMarker& marker,
  const SliderBackground& background,
  const IntSize& initialSize,
  const std::function<void(int)>& onChange);

} // namespace

namespace faint{ namespace events{

void on_slider_change(Slider*, const int_func&);
void on_slider_change(window_t, const int_func&);

}} // namespace




#endif
