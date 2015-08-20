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

#include "app/app-context.hh"
#include "app/app-getter-util.hh"
#include "util/setting-id.hh"

namespace faint{

std::function<Paint()> bg_getter(AppContext& app){
  return [&app](){return app.GetToolSettings().Get(ts_Bg);};
}

change_tool_f tool_changer(AppContext& app){
  return [&app](ToolId id){app.SelectTool(id);};
}

} // namespace
