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

#include "app/canvas.hh"
#include "gui/grid-dialog.hh"

namespace faint{

void context_show_grid_dialog(Canvas& canvas, DialogContext& c){
  auto result = show_grid_dialog(nullptr, canvas.GetGrid(), c);
  result.IfSet([&](const Grid& grid){
    canvas.SetGrid(grid);
    canvas.Refresh();
  });
}

} // namespace
