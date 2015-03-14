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

#include "app/active-canvas.hh"
#include "tasks/select-raster-idle.hh"
#include "tools/multi-tool.hh"
#include "util/raster-selection.hh"
#include "util/setting-util.hh"

namespace faint{

class RasterSelectionTool : public MultiTool {
public:
  RasterSelectionTool(const Settings& allSettings, const ActiveCanvas& canvas)
    : MultiTool(ToolId::OTHER,
      default_task(raster_selection_idle_task(m_settings, canvas)))
  {
    const RasterSelection& selection = canvas->GetRasterSelection();
    m_settings = get_selection_settings(selection);
    if (!selection.Exists()){
      // If the selection is inactive, use the current background
      // color instead
       m_settings.Set(ts_Bg, allSettings.Get(ts_Bg));
    }
  }

  const Settings& GetSettings() const override{
    return m_settings;
  }

  bool Set(const BoundSetting& s) override{
    if (m_settings.Update(s)){
      UpdateTaskSettings();
      return true;
    }
    return false;
  }

  bool UpdateSettings(const Settings& s) override{
    if (m_settings.Update(s)){
      UpdateTaskSettings();
      return true;
    }
    return false;
  }
private:
  Settings m_settings;
};

Tool* raster_selection_tool(const Settings& all, const ActiveCanvas& canvas){
  return new RasterSelectionTool(all, canvas);
}

} // namespace
