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

#include "gui/paint-dialog/gradient-slider.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/layout-wx.hh"
#include "tests/test-util/pick-random-color.hh"

namespace faint{ class StatusInterface; }

namespace {
faint::LinearGradient linearGradient;
faint::RadialGradient radialGradient;
}

void gui_test_gradient_slider(wxWindow* p, faint::StatusInterface&,
  faint::DialogContext& c)
{
  using namespace faint;

  linearGradient = faint::LinearGradient(Angle::Zero(),
    {{color_red, 0.0}, {color_magenta, 1.0}});

  radialGradient = faint::RadialGradient(Point(0,0), Radii(5, 5),
    {{color_red, 0.0}, {color_magenta, 1.0}});

  // Fixme: Leaked. (Crash if LinearGradientSlider is destroyed).
   auto* linearSlider = new faint::LinearGradientSlider(p,
     IntSize(300, 200),
     linearGradient,
     test::random_color_picker(42),
     c);
   linearSlider->SetBackgroundColor({0,0,100});

   auto* radialSlider = new faint::RadialGradientSlider(p,
     IntSize(300, 200),
     radialGradient,
     test::random_color_picker(42),
     c);
   radialSlider->SetBackgroundColor({0,0,100});

   // TODO: Add gradient displays (linear-gradient-display.hh)

   set_sizer(p,
     layout::create_column({linearSlider->AsWindow(), radialSlider->AsWindow()}));
}
