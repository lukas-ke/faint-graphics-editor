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

#include "geo/canvas-geo.hh"
#include "geo/geo-func.hh"
#include "geo/int-point.hh"
#include "geo/int-rect.hh"
#include "geo/point.hh"
#include "util/mouse.hh"

namespace faint{namespace mouse {

Point view_to_image(const IntPoint& p, const CanvasGeo& g){
  return (p + g.pos - point_from_size(g.border)) / g.Scale();
}

IntPoint image_to_view(const IntPoint& p, const CanvasGeo& g){
  return floored(p * g.Scale()) - g.pos + point_from_size(g.border);
}

IntRect image_to_view(const IntRect& r, const CanvasGeo& g){
  return IntRect(image_to_view(r.TopLeft(), g),
    image_to_view(r.BottomRight(), g));
}

}} // namespace
