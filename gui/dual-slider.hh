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

#ifndef FAINT_DUAL_SLIDER_HH
#define FAINT_DUAL_SLIDER_HH
#include "wx/event.h"
#include "wx/panel.h"
#include "geo/range.hh"
#include "gui/slider-common.hh"
#include "util-wx/bind-event.hh"
#include "util-wx/window-types-wx.hh"

namespace faint{

class DualSlider : public wxPanel{
  // Interface for sliders with two handles defining an interval
public:
  DualSlider(wxWindow* parent);
  virtual Interval GetSelectedInterval() const = 0;
  virtual void SetSelectedInterval(const Interval&) = 0;
  virtual void SetBackground(const SliderBackground&) = 0;
private:
  template<int MINB, int MAXB, typename FUNC>
  friend DualSlider* create_dual_slider(wxWindow*,
    const StaticBoundedInterval<MINB, MAXB>&,
    const SliderBackground&,
    const IntSize&,
    const FUNC&);

  template<int MINB, int MAXB>
  friend DualSlider* create_dual_slider(wxWindow*,
    const StaticBoundedInterval<MINB, MAXB>&,
    const SliderBackground&,
    const IntSize&);

  static DualSlider* Create(wxWindow* parent,
    const ClosedIntRange&,
    const Interval&,
    const SliderBackground&,
    const IntSize& size);
};

namespace events {

using interval_func = std::function<void(const Interval&, bool /*special*/)>;
void on_slider_change(DualSlider*, const interval_func&);
void on_dual_slider_change(window_t, const interval_func&);

} // namespace

// Creates a dual slider control
template<int MINB, int MAXB>
DualSlider* create_dual_slider(wxWindow* parent,
  const StaticBoundedInterval<MINB, MAXB>& values,
  const SliderBackground& bg,
  const IntSize& initialSize)
{
  using RangeType = StaticBoundedInterval<MINB, MAXB>;
  return DualSlider::Create(parent,
    make_closed_range(RangeType::min_allowed, RangeType::max_allowed),
    values.GetInterval(),
    bg,
    initialSize);
}

// Creates and binds a dual slider control
template<int MINB, int MAXB, typename FUNC>
DualSlider* create_dual_slider(wxWindow* parent,
  const StaticBoundedInterval<MINB, MAXB>& values,
  const SliderBackground& bg,
  const IntSize& initialSize,
  const FUNC& onChange)
{
  using RangeType = StaticBoundedInterval<MINB, MAXB>;
  auto s = DualSlider::Create(parent,
    make_closed_range(RangeType::min_allowed, RangeType::max_allowed),
    values.GetInterval(),
    bg,
    initialSize);
  events::on_slider_change(s, onChange);
  return s;
}

} // namespace


#endif
