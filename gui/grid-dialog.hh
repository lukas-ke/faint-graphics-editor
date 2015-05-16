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

#ifndef FAINT_GRID_DIALOG_HH
#define FAINT_GRID_DIALOG_HH
#include "gui/dialog-context.hh"
#include "util/grid.hh"
#include "util/optional.hh"

class wxWindow;

namespace faint{

Optional<Grid> show_grid_dialog(wxWindow* parent, const Grid&, DialogContext&);

} // namespace

#endif
