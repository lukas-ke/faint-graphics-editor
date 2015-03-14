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

#ifndef FAINT_COMMAND_CONSTANTS_HH
#define FAINT_COMMAND_CONSTANTS_HH
#include "util/distinct.hh"

namespace faint{

// The command type informs if a command affects the raster layer,
// objects or both objects and the raster layer.
enum class CommandType{RASTER, OBJECT, HYBRID, SELECTION, FRAME};

class category_command;
using select_added = Distinct<bool, category_command, 0>;
using deselect_old = Distinct<bool, category_command, 1>;

} // namespace

#endif
