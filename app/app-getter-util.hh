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

#include <functional>
#include "bitmap/paint.hh"
#include "tools/tool-id.hh"

namespace faint{

class AppContext;
enum class ToolId;

// Functions to simplify passing functions to access parts of an
// AppContext.
//
// Introduced to simplify transition away from using get_app_context
// in the deepest bowels of Faint, in favor of passing lambdas and
// such.

std::function<Paint()> bg_getter(AppContext&);
std::function<void(ToolId)> tool_changer(AppContext&);


}
