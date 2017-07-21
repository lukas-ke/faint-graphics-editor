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
#include "geo/range.hh"
#include "gui/slider-common.hh"
#include "util-wx/fwd-bind.hh"

namespace faint{

class Slider : public wxPanel{
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
  virtual void Draw(Bitmap&, SliderDir, const IntSize&, int pos) const = 0;
};

using SliderMarkerPtr = std::unique_ptr<SliderMarker>;

// A line indicating a slider position.
SliderMarkerPtr create_LineSliderMarker();

// A bordered line indicating a slider position.
SliderMarkerPtr create_BorderedSliderMarker();

// Creates a horizontal or vertical slider control
Slider* create_slider(wxWindow* parent,
  const BoundedInt&,
  SliderDir,
  SliderMarkerPtr,
  const SliderBackground&,
  const SliderCursors&,
  const IntSize& initialSize);

Slider* create_slider(wxWindow* parent,
  const BoundedInt&,
  SliderDir,
  const SliderBackground&,
  const SliderCursors&,
  const IntSize& initialSize);

Slider* create_slider(wxWindow* parent,
  const BoundedInt& values,
  SliderDir dir,
  SliderMarkerPtr,
  const SliderBackground&,
  const SliderCursors&,
  const IntSize& initialSize,
  const std::function<void()>& onChange);

Slider* create_slider(wxWindow* parent,
  const BoundedInt& values,
  SliderDir dir,
  const SliderBackground&,
  const SliderCursors&,
  const IntSize& initialSize,
  const std::function<void()>& onChange);

Slider* create_slider(wxWindow* parent,
  const BoundedInt& values,
  SliderDir,
  SliderMarkerPtr,
  const SliderBackground&,
  const SliderCursors&,
  const IntSize&,
  const std::function<void(int)>& onChange);

} // namespace

namespace faint{ namespace events{

void on_slider_change(Slider*, const int_func&);
void on_slider_change(window_t, const int_func&);

}} // namespace

#endif
