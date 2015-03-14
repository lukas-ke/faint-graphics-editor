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

#include <cassert>
#include "tools/tool-id.hh"
#include "util/setting-id.hh"

namespace faint{

bool raster_layer_only(ToolId id){
  return id == ToolId::PEN ||
    id == ToolId::BRUSH;
}

int to_int(ToolId id){
  return static_cast<int>(id);
}

ToolId to_tool_id(int id){
  assert(valid_tool_id(id));
  return static_cast<ToolId>(id);
}

bool valid_tool_id(int id){
  return (0 <= to_int(ToolId::MIN_VALUE) && id <= to_int(ToolId::MAX_VALUE));
}

Layer get_tool_layer(ToolId tool, Layer defaultLayer){
  if (raster_layer_only(tool)){
    return Layer::RASTER;
  }
  return defaultLayer;
}

Tool* brush_tool(const Settings&);
Tool* calibrate_tool();
Tool* ellipse_tool(const Settings&);
Tool* fill_tool(const Settings&);
Tool* hot_spot_tool();
Tool* level_tool(const Settings&);
Tool* line_tool(const Settings&);
Tool* path_tool(const Settings&);
Tool* pen_tool(const Settings&);
Tool* picker_tool();
Tool* polygon_tool(const Settings&);
Tool* rectangle_tool(const Settings&);
Tool* selection_tool(Layer, const Settings&, const ActiveCanvas&);
Tool* spline_tool(const Settings&);
Tool* tape_measure_tool(const Settings&);
Tool* text_tool(const Settings&);


Tool* new_tool(ToolId id,
  Layer layer,
  const Settings& settings,
  const ActiveCanvas& canvas)
{
  switch (id) {
  case ToolId::BRUSH:
    return brush_tool(settings);

  case ToolId::ELLIPSE:
    return ellipse_tool(settings);

  case ToolId::FLOOD_FILL:
    return fill_tool(settings);

  case ToolId::LEVEL:
    return level_tool(settings);

  case ToolId::LINE:
    return line_tool(settings);

  case ToolId::SELECTION:
    return selection_tool(layer, settings, canvas);

  case ToolId::OTHER:
    assert(false); // Can not instantiate unspecific tool
    return nullptr;

  case ToolId::PEN:
    return pen_tool(settings);

  case ToolId::PATH:
    return path_tool(settings);

  case ToolId::PICKER:
    return picker_tool();

  case ToolId::POLYGON:
    return polygon_tool(settings);

  case ToolId::RECTANGLE:
    return rectangle_tool(settings);

  case ToolId::SPLINE:
    return spline_tool(settings);

  case ToolId::TEXT:
    return text_tool(settings);

  case ToolId::HOT_SPOT:
    return hot_spot_tool();

  case ToolId::CALIBRATE:
    return calibrate_tool();

  case ToolId::TAPE_MEASURE:
    return tape_measure_tool(settings);

  default:
    assert(false);
    return nullptr;
  };
}

} // namespace
