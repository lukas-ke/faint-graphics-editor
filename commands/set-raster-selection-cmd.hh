// -*- coding: us-ascii-unix -*-
// Copyright 2012 Lukas Kemmer
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

#ifndef FAINT_SET_RASTER_SELECTION_CMD_HH
#define FAINT_SET_RASTER_SELECTION_CMD_HH
#include "util/distinct.hh"
#include "util/raster-selection.hh"

namespace faint{
class Command;
class utf8_string;

// Lightweight command for moving a selection - does not cause another
// copy of the selected content.
Command* move_raster_selection_command(const IntPoint& newPos,
  const IntPoint& oldPos);

// Fixme: Replace these (stamp_floating) with a function taking a
// sel::Floating. This requires removing the filling of OldRect for
// the Copying-case, but that should probably be in deselect, not
// stamp.

// Stamps the content of a floating selection to the image. Does not
// modify the selection.
Command* stamp_floating_selection_command(const sel::Copying&);

// Stamps the content of a floating selection to the image, and
// fills the old source rectangle the selection was moved from.
// Does not modify the selection.
Command* stamp_floating_selection_command(const sel::Moving&);

Command* set_selection_options_command(const NewSelectionOptions&,
  const OldSelectionOptions&);

Command* set_raster_selection_command(const NewSelectionState&,
  const OldSelectionState&,
  const utf8_string& name,
  bool appendCommand=false);

Command* set_raster_selection_command(const NewSelectionState&,
  const Alternative<SelectionState>&,
  const OldSelectionState&,
  const utf8_string& name,
  bool appendCommand=false);

Command* set_raster_selection_command(const NewSelectionState&,
  const OldSelectionState&,
  const utf8_string& name,
  bool appendCommand,
  const NewSelectionOptions&,
  const OldSelectionOptions&
);

bool is_move_raster_selection_command(Command*);
bool is_appendable_raster_selection_command(Command*);

} // namespace

#endif
