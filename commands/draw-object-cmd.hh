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

#ifndef FAINT_DRAW_OBJECT_CMD_HH
#define FAINT_DRAW_OBJECT_CMD_HH
#include "util/distinct.hh"

namespace faint{
class Command;
class Object;

// Command for drawing the object (as raster graphics).
// Assumes ownership, will delete the object on destruction.
Command* draw_object_command(const its_yours_t<Object>&);

// Does not delete the object.
Command* draw_object_command(const just_a_loan_t<Object>&);

}

#endif
