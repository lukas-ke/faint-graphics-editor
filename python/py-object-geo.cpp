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

#include "app/canvas.hh"
#include "geo/measure.hh"
#include "geo/pathpt.hh"
#include "objects/object.hh"
#include "python/bound-object.hh"
#include "python/py-object-geo.hh"
#include "python/py-function-error.hh"
#include "util/image.hh"
#include "util/image-util.hh"
#include "util/iter.hh"
#include "util/object-util.hh"

namespace faint{

/* function: "perimeter()\n
Returns the perimeter length of the object.\n
Warning: Treats curves as lines currently." */
extern coord perimeter_py(const BoundObject<Object>& obj){
  const auto& frame = obj.canvas->GetFrame(obj.frameId);
  const coord conversion(get_pixel_to_mm_conversion(frame).Or(1.0));
  const coord pixels = perimeter(obj.obj, frame.GetExpressionContext());
  return pixels * conversion;
}

} // namespace
