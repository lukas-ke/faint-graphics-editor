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

#include "bitmap/bitmap-exception.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "gui/paint-dialog/pattern-panel.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/layout-wx.hh"

class wxWindow;
namespace faint{ class StatusInterface; }
namespace faint{ class DialogContext; }

void gui_test_pattern_panel(wxWindow* p,
  faint::StatusInterface&,
  faint::DialogContext&)
{
  using namespace faint;

  auto getBitmapGood = [](){
    return Bitmap({10, 10}, color_magenta);
  };

  auto getBitmapBad = []() -> faint::Bitmap{
    throw BitmapOutOfMemory("Oh no");
  };

  set_sizer(p,
    layout::create_column({
      create_label(p, "Functioning PatternPanel"),
      faint::PaintPanel_Pattern(p, getBitmapGood).AsWindow(),
      create_label(p, "Exceptions in PatternPanel"),
      faint::PaintPanel_Pattern(p, getBitmapBad).AsWindow()}));
}
