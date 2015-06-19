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

#ifndef FAINT_PIXEL_SNAP_HH
#define FAINT_PIXEL_SNAP_HH
#include "geo/tri.hh"
#include "geo/point.hh"

namespace faint{

// Functions for aligning line edges with the pixel grid,
// to avoid a smeared appearance.

Point pixel_snap_xy(const Point&, coord lineWidth);
Tri pixel_snap(const Tri&, coord lineWidth);
class Points;
Points pixel_snap(const Points&, coord lineWidth);

} // namespace

#endif
