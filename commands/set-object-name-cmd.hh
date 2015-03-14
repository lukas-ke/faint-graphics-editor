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

#ifndef FAINT_SET_OBJECT_NAME_CMD_HH
#define FAINT_SET_OBJECT_NAME_CMD_HH
#include "text/utf8-string.hh"
#include "util/template-fwd.hh"

namespace faint{

class Command;
class Object;

// Returns {} if the passed in optional is not set or set to an empty
// string.
Optional<utf8_string> empty_to_unset(const Optional<utf8_string>&);

Command* set_object_name_command(Object*, const Optional<utf8_string>&);

} // namespace

#endif
