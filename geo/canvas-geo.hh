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

#ifndef FAINT_CANVAS_GEO_HH
#define FAINT_CANVAS_GEO_HH
#include "geo/int-point.hh"
#include "geo/int-size.hh"
#include "util/zoom-level.hh"

namespace faint{

class CanvasGeo {
public:
  CanvasGeo() :
    border(20,20),
    pos(0,0)
  {}

  IntPoint Pos() const{
    return pos;
  }

  coord Scale() const{
    return zoom.GetScaleFactor();
  }

  void SetPos(const IntPoint& p){
    pos = p;
  }
  IntSize border;
  IntPoint pos;
  ZoomLevel zoom;
};

} // namespace

#endif
