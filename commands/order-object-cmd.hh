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

#ifndef FAINT_ORDER_OBJECT_CMD_HH
#define FAINT_ORDER_OBJECT_CMD_HH
#include "util/distinct.hh"

namespace faint{

class Command;
class Object;
class utf8_string;

using NewZ = Order<int>::New;
using OldZ = Order<int>::Old;

// Returns "Forward" or "Back"
utf8_string forward_or_back_str(const NewZ&, const OldZ&);

// Command which changes the Z-order of the object to NewZ. OldZ is
// used for undo.
Command* order_object_command(Object*, const NewZ&, const OldZ&);

} // namespace

#endif
