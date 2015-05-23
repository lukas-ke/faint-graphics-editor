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

#include "app/get-art.hh"
#include "geo/int-point.hh"
#include "gui/art.hh"
#include "gui/image-toggle-ctrl.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/fwd-wx.hh"
#include "util/setting-id.hh"

namespace faint{ class StatusInterface; }
namespace faint{ class DialogContext; }

void gui_test_image_toggle_ctrl(wxWindow* p, faint::StatusInterface& status,
  faint::DialogContext&)
{
  using namespace faint;
  auto& art = get_art();

  const IntSize bitmapSize(28, 23);
  {
    create_image_toggle(p,
      ts_BrushShape,
      bitmapSize,
      status,
      Tooltip("Whatever"),
      {ToggleImage(art.Get(Icon::BRUSH_CIRCLE), 1, "Circle"),
       ToggleImage(art.Get(Icon::BRUSH_RECT), 0, "Rect"),
       ToggleImage(art.Get(Icon::BRUSH_EXPERIMENTAL), 2, "Experimental")},
      Axis::VERTICAL);
  }
}
