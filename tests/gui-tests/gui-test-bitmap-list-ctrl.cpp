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

#include "app/get-art-container.hh"
#include "gui/art-container.hh"
#include "gui/bitmap-list-ctrl.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/fwd-wx.hh"
#include "geo/int-point.hh"

namespace faint{ class StatusInterface; }
namespace faint{ class DialogContext; }

void gui_test_bitmap_list_ctrl(wxWindow* p, faint::StatusInterface& status,
  faint::DialogContext&)
{
  using namespace faint;
  auto& art = get_art_container();
  const IntSize bitmapSize(to_faint(art.Get(Icon::BRUSH_RECT).GetSize()));
  {
    auto c = new BitmapListCtrl(p,
      bitmapSize,
      status,
      Axis::HORIZONTAL);
    c->Add(art.Get(Icon::BRUSH_CIRCLE), "Circle");
    c->Add(art.Get(Icon::BRUSH_RECT), "Square");
    c->Add(art.Get(Icon::BRUSH_EXPERIMENTAL), "Blurry");
    set_pos(c, {10, 10});
  }

  {
    auto c = new BitmapListCtrl(p,
      bitmapSize,
      status,
      Axis::VERTICAL);
    c->Add(art.Get(Icon::BRUSH_CIRCLE), "Circle");
    c->Add(art.Get(Icon::BRUSH_RECT), "Square");
    c->Add(art.Get(Icon::BRUSH_EXPERIMENTAL), "Blurry");
    set_pos(c, {10, 60});
  }
}
