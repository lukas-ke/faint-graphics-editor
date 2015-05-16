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

#include <string>
#include "geo/int-point.hh"
#include "geo/int-size.hh"
#include "gui/grid-dialog.hh"
#include "gui/ui-constants.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/layout-wx.hh"
#include "util-wx/gui-util.hh" // Fixme: for fit_size_to, not here!
#include "geo/geo-func.hh" // floated

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
  const Grid& grid,
  DialogContext& c)
{

  auto dlg = create_dialog(parent, "Grid");

  using namespace layout;

  auto labelSpacing = create_label(dlg.get(), "&Spacing", TextAlign::RIGHT);
  auto labelX0 = create_label(dlg, "&X0", TextAlign::RIGHT);

  auto editSpacing = create_text_control(dlg, "");
  set_number_text(editSpacing, grid.Spacing(), Signal::NO);
  fit_size_to(editSpacing, "10000");
  auto spacingRow = create_row(OuterSpacing(0), ItemSpacing(item_spacing),
    {labelSpacing, raw(editSpacing)});

  auto anchor = grid.Anchor();
  auto editX0 = create_text_control(dlg.get(), "");
  fit_size_to(editX0, "10000");
  set_number_text(editX0, anchor.x, Signal::NO);

  auto editY0 = create_text_control(dlg.get(), "");
  fit_size_to(editY0, "10000");
  set_number_text(editY0, anchor.y, Signal::NO);

  auto anchorRow = create_row(OuterSpacing(0), ItemSpacing(item_spacing),
    {labelX0, raw(editX0),
     create_label(dlg.get(), "&Y0"), raw(editY0)});
  auto dashes = create_checkbox(dlg, "&Dashes", grid.Enabled());
  auto dashesRow = create_row({raw(dashes)});
  auto okCancel = center(create_row({create_ok_cancel_buttons(dlg.get())}));
  make_uniformly_sized({labelSpacing, labelX0});

  set_sizer(dlg.get(), create_column({
   spacingRow,
   anchorRow,
   dashesRow,
   okCancel}));

  if (c.ShowModal(*dlg) == wxID_OK){ // Fixme: Use own enum. Change in DialogContext
    auto g = Grid(true,
      to_int(get_text(editSpacing), grid.Spacing()),
      default_grid_color(),  // Fixme
      floated(IntPoint(to_int(get_text(editX0), anchor.x),
          to_int(get_text(editY0), anchor.y))));
    g.SetDashed(get(dashes));
    return option(g);
  }
  return {};
}

} // namespace
