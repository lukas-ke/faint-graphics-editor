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

#include "wx/wx.h"
#include "bitmap/color.hh"
#include "bitmap/paint.hh"
#include "geo/int-point.hh"
#include "gui/dialog-context.hh"
#include "gui/palette-ctrl.hh"
#include "util-wx/fwd-wx.hh"
#include "util/paint-map.hh"
#include "util/setting-id.hh"
#include "util/optional.hh"

void gui_test_palette_ctrl(wxWindow* p,
  faint::StatusInterface& status,
  faint::DialogContext&)
{
  using namespace faint;

  PaintMap paintMap;
  paintMap.Append(Paint(Color(255,0,255)));
  paintMap.Append(Paint(Color(255,0,0)));
  paintMap.Append(Paint(Color(255,255,255)));
  paintMap.Append(Paint(Color(0,0,0)));
  paintMap.Append(Paint(Color(0,0,0, 100)));

  const auto pickPaint = [](const auto&, const auto&, const auto&){
    using namespace faint;
    return option(Paint(color_magenta));
  };

  const auto getSecondary = [](){return color_magenta;};

  PaletteCtrl c1(p, paintMap, status, pickPaint, getSecondary);
  set_pos(c1.AsWindow(), {10,10});

  // Another palette control for testing drag and drop between controls.
  PaletteCtrl c2(p, paintMap, status, pickPaint, getSecondary);
  set_pos(c2.AsWindow(), {10, 100});
}
