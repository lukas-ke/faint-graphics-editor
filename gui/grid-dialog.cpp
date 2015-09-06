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

#include <string> // std::stod
#include "bitmap/bitmap.hh"
#include "geo/geo-func.hh" // floated
#include "geo/int-point.hh"
#include "geo/int-size.hh"
#include "gui/accelerator-entry.hh"
#include "gui/grid-dialog.hh"
#include "gui/paint-dialog.hh"
#include "gui/static-bitmap.hh"
#include "gui/ui-constants.hh"
#include "util/color-bitmap-util.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/fwd-bind.hh"
#include "util-wx/layout-wx.hh"
#include "util-wx/key-codes.hh"

namespace faint{

static coord to_coord(const utf8_string& s, coord defaultValue){
  try{
    return std::stod(s.c_str());
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
  auto dlg = fixed_size_dialog(parent, "Grid");

  auto make_edit = [&](coord value){
    auto edit = create_text_control(dlg.get(), "");
    fit_size_to(edit, "10000");
    set_number_text(edit, value, 2_dec, Signal::NO);
    return edit;
  };

  // Spacing edit field
  auto labelSpacing = create_label(dlg.get(), "&Spacing", TextAlign::RIGHT);
  auto editSpacing = make_edit(grid.Spacing());

  // Anchor edit fields
  Point anchor = grid.Anchor();

  auto labelX = create_label(dlg, "&X", TextAlign::RIGHT);
  make_uniformly_sized({labelSpacing, labelX});
  auto editX = make_edit(anchor.x);

  auto labelY = create_label(dlg.get(), "&Y");
  auto editY = make_edit(anchor.y);

  // Enabled-checkbox
  auto enabled = create_checkbox(dlg, "&enabled", grid.Enabled());

  // Dashes-checkbox
  auto dashed = create_checkbox(dlg, "&dashes", grid.Dashed());

  using namespace layout;
  auto spacingRow = create_row(OuterSpacing(0), ui::item_spacing,
    {labelSpacing, raw(editSpacing)});

  auto anchorRow = create_row(OuterSpacing(0), ui::item_spacing,
    {labelX, raw(editX),
     labelY, raw(editY)});

  auto make_color_bitmap = [&](){
    return color_bitmap(grid.GetColor(), IntSize(20, 20));
  };

  auto colorLabel = create_label(dlg, "&color");
  auto colorButton = new StaticBitmap(raw(dlg), make_color_bitmap());
  set_size(colorButton, get_size(dashed));
  set_stock_cursor(colorButton, wxCURSOR_HAND);
  set_stock_cursor(colorLabel, wxCURSOR_HAND);

  auto pick_grid_color =  [&](){
    show_color_only_dialog(raw(dlg), "Grid color",
      grid.GetColor(), c).Visit(
        [&](const Color& c){
          grid.SetColor(c);
          colorButton->SetBitmap(make_color_bitmap());
        });
  };

  set_accelerators(raw(dlg), {
   {key::C, Alt+key::C, pick_grid_color}});

  events::on_mouse_left_down(colorButton,
    [&](const IntPoint&){
      pick_grid_color();
    });

  events::on_mouse_left_down(colorLabel,
    [&](const IntPoint&){
      pick_grid_color();
    });

  set_sizer(dlg.get(),
    create_column({
      create_row({raw(enabled)}),
      spacingRow,
      anchorRow,
      create_row({raw(dashed)}),
      create_row({raw(colorButton), colorLabel}),
      center(create_row({create_ok_cancel_buttons(dlg.get())}))}));

  auto get_grid = [&](){
    return Grid(enabled_t(get(enabled)),
      dashed_t(get(dashed)),
      to_coord(get_text(editSpacing), grid.Spacing()),
      grid.GetColor(),
      Point(to_coord(get_text(editX), anchor.x), to_coord(get_text(editY), anchor.y)));

  };

  center_over_parent(dlg);
  return c.ShowModal(*dlg) == DialogChoice::OK ?
    option(get_grid()) : no_option();
}

} // namespace
