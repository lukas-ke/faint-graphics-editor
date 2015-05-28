// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
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

#include "bitmap/color.hh"
#include "bitmap/paint.hh"
#include "geo/int-point.hh"
#include "geo/int-size.hh"
#include "gui/selected-color-ctrl.hh"
#include "util-wx/fwd-wx.hh"
#include "util/color-choice.hh"
#include "util/status-interface.hh"
#include "gui/dialog-context.hh"

void gui_test_selected_color_ctrl(wxWindow* p,
  faint::StatusInterface& status, faint::DialogContext&)
{
  const auto pickPaint = [](const auto&, const auto&, const auto&){
    using namespace faint;
    return option(Paint(color_magenta));
  };

  const auto getSecondary = [](){return faint::color_magenta;};

  using namespace faint;
  SelectedColorCtrl c1(p, IntSize(64, 64), status, pickPaint, getSecondary);
  c1.UpdateColors(ColorChoice({Paint(Color(128,0,100,50)), false},
      {Paint(Color(255,0,255)), false}));
  set_pos(c1.AsWindow(), {10,10});

  SelectedColorCtrl c2(p, IntSize(64, 64), status, pickPaint, getSecondary);
  c2.UpdateColors(ColorChoice({Paint(Color(128,0,100,50)), true},
      {Paint(Color(255,0,255)), false}));
  set_pos(c2.AsWindow(), {10, 84});

  SelectedColorCtrl c3(p, IntSize(64, 64), status, pickPaint, getSecondary);
  c3.UpdateColors(ColorChoice({Paint(Color(128,0,100,50)), false},
      {Paint(Color(255,0,255)), true}));
  set_pos(c3.AsWindow(), {10, 84 + 74});
}
