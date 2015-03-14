// -*- coding: us-ascii-unix -*-
// Copyright 2013 Lukas Kemmer
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

#include "app/canvas.hh"
#include "geo/rect.hh"
#include "geo/size.hh"
#include "util/apply-target.hh"
#include "util/object-util.hh"
#include "util/raster-selection.hh"
#include "util/setting-id.hh"

namespace faint{

ApplyTarget get_apply_target(const Canvas& canvas){
  const ToolInterface& tool(canvas.GetTool());
  Layer layer = tool.GetLayerType();
  if (layer == Layer::RASTER && canvas.GetRasterSelection().Exists()){
    return ApplyTarget::APPLY_RASTER_SELECTION;
  }
  else if (layer == Layer::OBJECT && !canvas.GetObjectSelection().empty()){
    return ApplyTarget::APPLY_OBJECT_SELECTION;
  }
  return ApplyTarget::APPLY_IMAGE;
}

EitherSize get_apply_target_size(const Canvas& canvas, ApplyTarget target){
  return dispatch_target(target,
    [&](OBJECT_SELECTION) ->  EitherSize{
      return bounding_rect(canvas.GetObjectSelection()).GetSize();
    },
    [&](RASTER_SELECTION) -> EitherSize{
      return canvas.GetRasterSelection().GetSize();
    },
    [&](IMAGE) -> EitherSize{
      return canvas.GetSize();
    });
}

} // namespace
