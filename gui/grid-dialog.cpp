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

#include <string> // std::stoi
#include "bitmap/bitmap.hh"
#include "geo/geo-func.hh" // floated
#include "geo/int-point.hh"
#include "geo/int-size.hh"
#include "gui/grid-dialog.hh"
#include "gui/paint-dialog.hh"
#include "gui/static-bitmap.hh"
#include "gui/ui-constants.hh"
#include "util/color-bitmap-util.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/fwd-bind.hh"
#include "util-wx/layout-wx.hh"

namespace faint{

static int to_int(const utf8_string& s, int defaultValue){
  try{
    return std::stoi(s.c_str());
  }
  catch (const std::exception&){
    return defaultValue;
  }
}

Optional<Grid> show_grid_dialog(wxWindow* parent,
  const Grid& oldGrid,
  DialogContext& c)
{
  Grid grid = oldGrid;
  auto dlg = create_fixed_size_dialog(parent, "Grid");

  auto make_edit = [&](int value){
    auto edit = create_text_control(dlg.get(), "");
    fit_size_to(edit, "10000");
    set_number_text(edit, value, Signal::NO);
    return edit;
  };

  // Spacing edit field
  auto labelSpacing = create_label(dlg.get(), "&Spacing", TextAlign::RIGHT);
  auto editSpacing = make_edit(grid.Spacing());

  // Anchor edit fields
  auto anchor = rounded(grid.Anchor()); // Fixme: Rounded, eh?

  auto labelX = create_label(dlg, "&X", TextAlign::RIGHT);
  make_uniformly_sized({labelSpacing, labelX});
  auto editX = make_edit(anchor.x);

  auto labelY = create_label(dlg.get(), "&Y");
  auto editY = make_edit(anchor.y);

  // Dashes-checkbox
  auto dashed = create_checkbox(dlg, "&Dashed lines", grid.Dashed());

  using namespace layout;
  auto spacingRow = create_row(OuterSpacing(0), ui::item_spacing,
    {labelSpacing, raw(editSpacing)});

  auto anchorRow = create_row(OuterSpacing(0), ui::item_spacing,
    {labelX, raw(editX),
     labelY, raw(editY)});

  auto make_color_bitmap = [&](){
    return color_bitmap(grid.GetColor(), IntSize(20, 20));
  };

  auto colorLabel = create_label(dlg, "Line color");
  auto colorButton = new StaticBitmap(raw(dlg.get()), make_color_bitmap());

  events::on_mouse_left_down(colorButton,
    [&](const IntPoint&){
      show_color_only_dialog(raw(dlg.get()), "Grid color",
        grid.GetColor(), c).Visit(
          [&](const Color& c){
            grid.SetColor(c);
            colorButton->SetBitmap(make_color_bitmap());
          });
    });

  set_sizer(dlg.get(),
    create_column({
      spacingRow,
      anchorRow,
      create_row({raw(dashed)}),
      create_row({raw(colorButton), colorLabel}),
      center(create_row({create_ok_cancel_buttons(dlg.get())}))}));

  auto get_grid = [&](){
    auto g = Grid(true,
      to_int(get_text(editSpacing), grid.Spacing()),
      grid.GetColor(),
      floated(IntPoint(to_int(get_text(editX), anchor.x),
        to_int(get_text(editY), anchor.y))));
    g.SetDashed(get(dashed));
    return g;
  };

  center_over_parent(dlg);
  return c.ShowModal(*dlg) == DialogChoice::OK ?
    option(get_grid()) : no_option();
}

} // namespace
